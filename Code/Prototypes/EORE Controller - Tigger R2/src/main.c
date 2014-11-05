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


#define DELAY_INTERVAL   250
#define TWI_SPEED        400000

void setup(void);
void send_led_command(uint8_t devAddr, uint8_t led, uint8_t brightness);


void setup(void)
{

	board_init();
	sysclk_init();
	ioport_init();
	wdt_disable(WDT);


	/* =============== UART1 =============== */ //(UART0 is defined but not UART1)
	#define PINS_DEVICE_TWI          (PIO_PA4A_TWCK0 | PIO_PA3A_TWD0)
	#define PINS_DEVICE_TWI_FLAGS    (PIO_PERIPH_A | PIO_DEFAULT)
	#define PINS_DEVICE_TWI_MASK     (PIO_PA4A_TWCK0 | PIO_PA3A_TWD0)
	#define PINS_DEVICE_TWI_PIO      PIOA
	#define PINS_DEVICE_TWI_ID       ID_PIOA
	#define PINS_DEVICE_TWI_TYPE     PIO_PERIPH_A
	#define PINS_DEVICE_TWI_ATTR     PIO_DEFAULT


	pio_configure_pin(PIO_PA3A_TWD0,  PIO_TYPE_PIO_PERIPH_A);
	pio_configure_pin(PIO_PA4A_TWCK0, PIO_TYPE_PIO_PERIPH_A);

	pio_configure(PINS_DEVICE_TWI_PIO, PINS_DEVICE_TWI_TYPE, PINS_DEVICE_TWI_MASK, PINS_DEVICE_TWI_ATTR);

	sysclk_enable_peripheral_clock(ID_TWI0);
	sysclk_enable_peripheral_clock(PINS_DEVICE_TWI_ID);
	sysclk_enable_peripheral_clock(ID_PIOB);


	twi_options_t conf;

	conf.master_clk = sysclk_get_cpu_hz();
	conf.speed = TWI_SPEED;
	conf.chip = 0;
	conf.smbus = 0;


	twi_master_init(DEVICE_TWI, &conf);

	ioport_set_pin_dir(LED_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(LED_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(LED_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(LED_4, IOPORT_DIR_OUTPUT);

	// ioport_set_pin_level(LED_DRIVER_RESET, 1);
	// ioport_set_pin_dir(LED_DRIVER_RESET, IOPORT_DIR_OUTPUT);


	// // SoftI2CInit();
	// ioport_set_pin_level(LED_DRIVER_RESET, 0);
	// delay_ms(5);
	// ioport_set_pin_level(LED_DRIVER_RESET, 1);






}


void send_led_command(uint8_t devAddr, uint8_t led, uint8_t brightness)
{
	twi_packet_t packet;
	volatile uint8_t status;

	/* Data to send */
	packet.buffer = &brightness;
	/* Data length */
	packet.length = 1;
	/* Slave chip address */
	packet.chip = (uint32_t) devAddr >> 1;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
	/* Internal chip address */
	packet.addr[0] = led;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_write(TWI0, &packet);

	// if (status == TWI_SUCCESS)
	// {
	// 	ioport_toggle_pin_level(LED_6);
	// }
	// else
	// {
	// 	ioport_toggle_pin_level(LED_5);
	// }

}

int main (void)
{
	setup();

	uint8_t dir = 0;
	uint8_t brightness = 0;
	while (1)
	{



		// for (uint8_t led = 0; led < 16; led += 1)
		// {
		// 	send_led_command(DRV_1_ADDR, led, brightness);
		// 	send_led_command(DRV_2_ADDR, led, brightness);
		// 	send_led_command(DRV_3_ADDR, led, brightness);
		// 	send_led_command(DRV_4_ADDR, led, brightness);
		// }
		// // Spin
		// ioport_set_pin_level(LED_1, 1);
		// delay_ms(DELAY_INTERVAL);
		// ioport_set_pin_level(LED_1, 0);

		// if (brightness == 0xFF)
		// {
		// 	dir = 1;
		// }
		// else if (brightness == 0x00)
		// {
		// 	dir = 0;
		// }

		// if (dir == 0)
		// {
		// 	brightness += 1;
		// }
		// else if (dir == 1)
		// {
		// 	brightness -= 1;
		// }


		ioport_set_pin_level(LED_1, 1);
		delay_ms(DELAY_INTERVAL);
		ioport_set_pin_level(LED_1, 0);

		ioport_set_pin_level(LED_2, 1);
		delay_ms(DELAY_INTERVAL);
		ioport_set_pin_level(LED_2, 0);

		ioport_set_pin_level(LED_3, 1);
		delay_ms(DELAY_INTERVAL);
		ioport_set_pin_level(LED_3, 0);

		ioport_set_pin_level(LED_4, 1);
		delay_ms(DELAY_INTERVAL);
		ioport_set_pin_level(LED_4, 0);


	}
	// Insert application code here, after the board has been initialized.
}
