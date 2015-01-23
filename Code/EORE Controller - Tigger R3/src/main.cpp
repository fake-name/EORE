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
 *
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
 #include <asf.h>
 #include <string.h>

#include "board/board.hpp"

#include "parse.hpp"

#include "print/print.hpp"
#include "si570/Si570.hpp"
#include "tmp/tmp100.hpp"
#include "spi/spi.hpp"
#include "pwm/pwm.hpp"



// #define DEBUG_PR(x ...)  // Default to NO debug
#define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug



// #########################################################
// Global variables
// #########################################################



ISR(HardFault_Handler)
{


	while (1) {
	}
}



int main (void)
{
	setup();

	uint16_t cnt = 0;

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

}
