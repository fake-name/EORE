/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
 #include <asf.h>
 #include <string.h>

#include "print/print.hpp"
#include "si570/Si570.hpp"
#include "spi/spi.hpp"
#include "board/board.hpp"

#define DELAY_INTERVAL   100
#define TWI_SPEED        400000

#define PACKET_HEADER 0x5D
#define WRITE_ATTEN   0xAA
#define WRITE_SWITCH  0x0C
#define WRITE_FREQ    0xC0
#define WRITE_MISC    0xDD

Si570 vfo = Si570(SI570_I2C_ADDRESS, 56.320e6);

// #########################################################
// State machine typedefs and variables
// #########################################################
typedef enum
{
	NO_PACKET,
	WAIT_COMMAND,
	WAIT_TGT,
	WAIT_DATA_1,
	WAIT_DATA_2,
	WAIT_DATA_3,
	WAIT_DATA_4,
	WAIT_CHECKSUM,

	PACKET_VALID
} sysState;

typedef union UnionU32_t
{
	uint8_t  bytes[4];
	uint32_t value;
} unionU32_t;

typedef struct
{
	uint8_t command;
	uint8_t target;
	unionU32_t value;
} command_packet;


volatile uint8_t checksum = 0;
volatile command_packet packet = {0, 0, 0};
volatile sysState state = NO_PACKET;


// #########################################################
// Local forward-definitions
// #########################################################
void setup(void);
void parse(uint8_t dataByte);
void process_packet(volatile command_packet *pkt);
void parse_misc(volatile command_packet *pkt);



// #########################################################
// State machine Functions
// #########################################################
void parse_misc(volatile command_packet *pkt)
{
	// We can assume that pkt->command is WRITE_MISC at this point, no need to re-check.
	switch (pkt->target)
	{
		case 0:  // Case 0 is control of the noise diode
			if (pkt->value.value == 0)
			{
				ioport_set_pin_level(NOISE_DIODE_PS, 0);	
			}
			else
			{
				ioport_set_pin_level(NOISE_DIODE_PS, 1);	
			}
			debugUnique("OK: Set noise diode powersupply: %i", pkt->value.value);
			break;
		default:
			debugUnique("ERROR: Unknown misc target val %i", pkt->target);
			break;
						
	}
}
void process_packet(volatile command_packet *pkt)
{
	// TODO: Break all the sub-command parse bits into separate functions.
	// Maybe inline them? Call overhead is probably pretty minor, not sure if worth bothering.
	
	Spi_Status tmp;
	Si570_Status vfo_tmp;
	switch (pkt->command)
	{
		case WRITE_ATTEN:
			tmp = writeAttenuator((uint8_t) pkt->target, (uint8_t) pkt->value.value);
			
			if (tmp == SET_SUCCESS)
			{
				debugUnique("OK: Atten Set %i -> %i", (uint8_t) pkt->target, pkt->value.value);
			}
			else
			{
				debugUnique("ERROR: Atten Set %i -> %i", (uint8_t) pkt->target, pkt->value.value);
			}

			break;


		case WRITE_SWITCH:
			tmp = writeSwitch((uint8_t) pkt->target, (uint8_t) pkt->value.value);
			
			
			if (tmp == SET_SUCCESS)
			{
				debugUnique("OK: Write Switch %i", pkt->value.value);
			}
			else
			{
				debugUnique("ERROR: Write Switch %i", pkt->value.value);
			}

			
			break;

		case WRITE_MISC:		
			parse_misc(pkt);
			break;

		case WRITE_FREQ:
			if (pkt->value.value == 0)
			{
				// A value of 0 disables the oscillator.
				ioport_set_pin_level(OSC_EN, 0);				
				
			}
			else if ((pkt->value.value < 10e6) | (pkt->value.value > 810e6))
			{
				debugUnique("ERROR: Invalid Frequency:  %i", pkt->value.value);
			}
			else if (pkt->target != 0)
			{
				debugUnique("ERROR: Invalid oscillator! Only one oscillator (0) currently supported:  %i", pkt->target);
			}
			else
			{
				// Ensure the oscillator is on before setting it.
				ioport_set_pin_level(OSC_EN, 1);
				
				
				vfo_tmp = vfo.setFrequency(pkt->value.value);
				if (vfo_tmp == SI570_SUCCESS)
				{
					debugUnique("OK: Freq Set:  %i", pkt->value.value);	
				}
				else
				{
					debugUnique("ERROR: Freq Set:  %i", pkt->value.value);	
				}
			}
		break;

		default:
			debugUnique("Error! Unknown command!");
			break;
	}


}


void parse(uint8_t dataByte)
{
	switch (state)
	{
		case NO_PACKET:
			if (dataByte == PACKET_HEADER)
			{
				state = WAIT_COMMAND;
				checksum = dataByte;
			}
			else
			{
				debugUnique("Not start byte! Wat?");
			}
			break;

		case WAIT_COMMAND:
			packet.command = dataByte;
			state = WAIT_TGT;
			checksum += dataByte;
			break;

		case WAIT_TGT:
			packet.target = dataByte;
			state = WAIT_DATA_1;
			checksum += dataByte;
			break;



		case WAIT_DATA_1:
			packet.value.bytes[0] = dataByte;
			state = WAIT_DATA_2;
			checksum += dataByte;
			break;
			
		case WAIT_DATA_2:
			packet.value.bytes[1] = dataByte;
			state = WAIT_DATA_3;
			checksum += dataByte;
			break;
		
		case WAIT_DATA_3:
			packet.value.bytes[2] = dataByte;
			state = WAIT_DATA_4;
			checksum += dataByte;
			break;
		
		case WAIT_DATA_4:

			packet.value.bytes[3] = dataByte;
			state = WAIT_CHECKSUM;
			checksum += dataByte;
			break;

		case WAIT_CHECKSUM:

			if (dataByte ==  checksum)
			{
				state = PACKET_VALID;
				process_packet(&packet);
			}
			else
			{
				
				debugUnique("Invalid checksum!");
				debugUnique("Should be  %i, received %i.", checksum, dataByte);
			}
			// Falls through to default (intentionally)!
		default:
			debugUnique("Rx Complete");
			state = NO_PACKET;
			checksum = 0;
			break;
	}

}




ISR(HardFault_Handler)
{

	ioport_set_pin_level(LED_4, 1);
	while (1) {
    }
}




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

	ioport_set_pin_dir(HEATER_ON, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(OSC_EN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(NOISE_DIODE_PS, IOPORT_DIR_OUTPUT);


	ioport_set_pin_level(HEATER_ON, 0);
	ioport_set_pin_level(OSC_EN, 1);
	ioport_set_pin_level(NOISE_DIODE_PS, 0);

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


	/* =============== SPI Setup =============== */

	//
	// SPI is done with soft-SPI routines because the DAT31R5-SP+ has
	// a weird interface that doesn't use any of the available common
	// SPI modes (it looks like it's level based, rather then edge-based
	// like /EVERY/ other SPI-like system ever).
	// Therefore, all the relevant pins are just set as outputs.
	//
	ioport_set_pin_dir(CS_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_4, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_5, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_OSC_ATTEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(CS_SW, IOPORT_DIR_OUTPUT);

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
	
	cpu_irq_enable();

}


int main (void)
{
	setup();

	
	vfo.initialize();
	vfo.setFrequency(100E6);
	
	

	for (int x = 0; x < 6; x += 1)
	{
		writeAttenuator(x, 20);		
	}	
	
	uint8_t led = 0;
	
	while (1)
	{	
		
		if (rxAvailable())
		{
			int tmp = rxRead();
			if (tmp >= 0)
			{
				parse(tmp);
			}
			
			if (led)
			{
				led = 0;
				
				ioport_set_pin_level(LED_1, 1);
			}
			else
			{
				led = 1;
				ioport_set_pin_level(LED_1, 0);
				
			}
			
		}
		
	}
	// Insert application code here, after the board has been initialized.
}
