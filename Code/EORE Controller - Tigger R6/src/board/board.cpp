/*
 * board.cpp
 *
 * Created: 11/6/2014 3:35:19 PM
 *  Author: Fake Name
 */


#include <asf.h>
#include "board/board.hpp"

#include "print/print.hpp"
#include "si570/Si570.hpp"
#include "tmp/tmp100.hpp"
#include "spi/spi.hpp"
#include "pwm/pwm.hpp"
#include "parse.hpp"
#include "dac/dac.h"




// #########################################################
// System Setup
// #########################################################
void setup(void)
{

	board_init();
	sysclk_init();
	ioport_init();
	wdt_disable(WDT);


	/* =============== General IO Setup =============== */

	//ioport_set_pin_dir(HEATER_OLD, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(HEATER_ON, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(OSC_EN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(NOISE_DIODE_PS, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(VCO_SWEEPER_PS, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(VCO_ON, IOPORT_DIR_OUTPUT);

	ioport_set_pin_level(HEATER_ON, 0);
	ioport_set_pin_level(OSC_EN, 1);
	ioport_set_pin_level(NOISE_DIODE_PS, 0);
	ioport_set_pin_level(VCO_SWEEPER_PS, 0);
	ioport_set_pin_level(VCO_ON, 0);
	
	ioport_set_pin_dir(LED_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(LED_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(LED_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(LED_4, IOPORT_DIR_OUTPUT);


	/* =============== Hook up peripherals to the proper pins =============== */

	// Enable the PMCs so shit actually does stuff.
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);

	// TWI Interface
	pio_configure_pin_group(PIOA, PIO_PA3A_TWD0,  PIO_TYPE_PIO_PERIPH_A);
	pio_configure_pin_group(PIOA, PIO_PA4A_TWCK0, PIO_TYPE_PIO_PERIPH_A);

	// Serial Interface
	pio_configure_pin_group(PIOB, PIO_PB10B_URXD3, PIO_TYPE_PIO_PERIPH_B);
	pio_configure_pin_group(PIOB, PIO_PB11B_UTXD3, PIO_TYPE_PIO_PERIPH_B);

	// PWM Interface
	pio_configure_pin_group(PIOA, PIO_PA2A_PWM2,  PIO_TYPE_PIO_PERIPH_A);


	/* =============== TWI Setup =============== */


	pio_configure(PINS_DEVICE_TWI_PIO, PINS_DEVICE_TWI_TYPE, PINS_DEVICE_TWI_MASK, PINS_DEVICE_TWI_ATTR);

	sysclk_enable_peripheral_clock(ID_TWI0);
	sysclk_enable_peripheral_clock(PINS_DEVICE_TWI_ID);

	twi_options_t twi_conf;

	twi_conf.master_clk = sysclk_get_cpu_hz();
	twi_conf.speed      = TWI_SPEED;
	twi_conf.chip       = 0;
	twi_conf.smbus      = 0;

	twi_master_init(DEVICE_TWI, &twi_conf);
	// twi_disable_slave_mode(DEVICE_TWI);
	// twi_enable_master_mode(DEVICE_TWI);


	/* =============== SPI Setup =============== */

	//
	// SPI is done with soft-SPI routines because the DAT31R5-SP+ has
	// a weird interface that doesn't use any of the available common
	// SPI modes (it looks like it's level based, rather then edge-based
	// like /EVERY/ other SPI-like system ever).
	// Therefore, all the relevant pins are just set as outputs.
	//
	// The shift register on the EORE board has a more traditional SPI 
	// interface, but the DAT31R5 pseudo-SPI mode will work fine with it
	// anyways, since it allows both rising and falling edge sampling
	// to work, as it uses a 4-phase-clocking-like mechanism.
	//
	ioport_set_pin_dir(CS_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_4, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_5, IOPORT_DIR_OUTPUT);  // This is the latch for the shift-register.
	ioport_set_pin_dir(CS_6, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_OSC_ATTEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_SWEEP_ATTEN, IOPORT_DIR_OUTPUT);

	ioport_set_pin_dir(MOSI, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(SCK, IOPORT_DIR_OUTPUT);


	/* =============== Debug UART Setup =============== */

	static usart_serial_options_t usart_options = {
		.baudrate   = DEBUG_UART_BAUDRATE,
		.charlength = DEBUG_UART_CHAR_LENGTH,
		.paritytype = DEBUG_UART_PARITY,
		.stopbits   = DEBUG_UART_STOP_BIT
	};
	sysclk_enable_peripheral_clock(DEBUG_UART_ID);
	usart_serial_init(DEBUG_UART, &usart_options);

	ioport_set_pin_dir(DEBUG_UART_PIN_TX, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(DEBUG_UART_PIN_RX, IOPORT_DIR_INPUT);

	usart_enable_tx(DEBUG_UART);
	usart_enable_rx(DEBUG_UART);

	usart_enable_interrupt(DEBUG_UART, US_IER_RXRDY);
	NVIC_EnableIRQ(UART3_IRQn);





	/* =============== PWM Setup =============== */


	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, PWM_CHANNEL_2);  // Precautionary disable.


	pwm_clock_t clock_setting;
	clock_setting.ul_clka = 1000;           // 1 Hz PWM(it's sloooooow)
	clock_setting.ul_clkb = 0;
	clock_setting.ul_mck = sysclk_get_cpu_hz();

	pwm_init(PWM, &clock_setting);

	pwm_channel_t pwm_channel_instance;

	pwm_channel_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_channel_instance.ul_period = 1000;
	pwm_channel_instance.ul_duty = 990;
	pwm_channel_instance.channel = PWM_CHANNEL_2;

	pwm_channel_init(PWM, &pwm_channel_instance);


	// Enable interrupt, and in the NVIC.
	pwm_channel_enable_interrupt(PWM, PWM_CHANNEL_2, 0);
	NVIC_EnableIRQ(PWM_IRQn);


	pwm_channel_enable(PWM, PWM_CHANNEL_2);

	/* =============== Call various subsystem-specific setups =============== */

	setupPwm();

	setup_vfo();
	
	enableDac();

	setup_all_tmp100();
	
	// Finally, turn on the interrupts.
	cpu_irq_enable();
}
