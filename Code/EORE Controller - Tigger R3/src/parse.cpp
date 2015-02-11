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

#include "parse.hpp"
#include "print/print.hpp"
#include "si570/Si570.hpp"
#include "tmp/tmp100.hpp"
#include "spi/spi.hpp"
#include "board/board.hpp"
#include "pwm/pwm.hpp"
#include "dac/dac.h"


// #define DEBUG_PR(x ...)  // Default to NO debug
#define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug


// #########################################################
// State machine global variables
// #########################################################


volatile uint8_t checksum = 0;
volatile command_packet packet = {0, 0, 0};
volatile sysState state = NO_PACKET;

Si570 vfo = Si570(SI570_I2C_ADDRESS, 56.320e6);


// #########################################################
// State machine convenience Functions
// #########################################################
inline void parse_misc(volatile command_packet *pkt)
{
	// We can assume that pkt->command is WRITE_MISC at this point, no need to re-check.
	switch (pkt->target)
	{
		case 0:  // Case 0 is control of the noise diode
			if (pkt->value.value == 0)
			{
				ioport_set_pin_level(NOISE_DIODE_PS, 0);
				debugUnique("OK: Set noise diode powersupply to OFF");
			}
			else
			{
				ioport_set_pin_level(NOISE_DIODE_PS, 1);
				debugUnique("OK: Set noise diode powersupply to ON");
			}
			break;
		case 1: // Case 1 is control of the VCO sweeper power.
				// Hardware support for this is currently not present on some boards.
			if (pkt->value.value == 0)
			{
				ioport_set_pin_level(VCO_SWEEPER_PS, 0);
				debugUnique("OK: Set noise VCO sweeper powersupply to OFF");
			}
			else
			{
				ioport_set_pin_level(VCO_SWEEPER_PS, 1);
				debugUnique("OK: Set noise VCO sweeper powersupply to ON");
			}
			break;
		default:
			debugUnique("ERROR: Unknown misc target val %i", pkt->target);
			break;

	}
}

inline void parse_temp(volatile command_packet *pkt)
{

	switch (pkt->target)
	{
		case 0:  // Case 0 is set temperature
			float temp;
			temp = 0.0625 * pkt->value.value;
			set_temperature(temp);
			debugUnique("OK: Current setpoint: %f", getTemperatureSetpoint());
			break;

		case 1:  // Get temperature. value parameter is ignored.
			debugUnique("OK: Current temperature: %f", getTemperature());
			break;

		case 2:  // Get setpoint. value parameter is ignored.
			debugUnique("OK: Setpoint temperature: %f", getTemperatureSetpoint());
			break;

		default:
			debugUnique("ERROR: Unknown temp target %i", pkt->target);
			break;

	}
}


inline void parse_switch(volatile command_packet *pkt)
{
	Spi_Status tmp;
	tmp = writeSwitch((uint8_t) pkt->target, (uint8_t) pkt->value.value);

	if (tmp == SET_SUCCESS)
	{
		debugUnique("OK: Write Switch %i -> %i", pkt->target, pkt->value.value);
	}
	else
	{
		debugUnique("ERROR: Write Switch %i -> %i", pkt->target, pkt->value.value);
	}
}

inline void parse_atten(volatile command_packet *pkt)
{
	Spi_Status tmp;
	tmp = writeAttenuator((uint8_t) pkt->target, (uint8_t) pkt->value.value);

	if (tmp == SET_SUCCESS)
	{
		debugUnique("OK: Atten Set %i -> %f (%i)", (uint8_t) pkt->target, (((float)pkt->value.value) / 0.5), pkt->value.value);
	}
	else
	{
		debugUnique("ERROR: Atten Set %i -> %i", (uint8_t) pkt->target, pkt->value.value);
	}
}

inline void parse_vfo(volatile command_packet *pkt)
{

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

		Si570_Status vfo_tmp;
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
}



inline void parse_sweeper(volatile command_packet *pkt)
{

	switch (pkt->target)
	{
		case 0:  // Case 0 is the main VCO sweeper
			if (pkt->value.value > 0)
			{
				
				ioport_set_pin_level(VCO_ON, 1);
				debugUnique("OK: Generating %i VCO chirps", pkt->value.value);
				sweep_chirp(pkt->value.value);
			}
			else
			{
				ioport_set_pin_level(VCO_ON, 0);
				debugUnique("OK: VCOs disabled");

			}
			break;
		default:
			debugUnique("ERROR: Unknown VCO chirp target %i", pkt->target);
			break;

	}
}


// #########################################################
// Process a complete and valid packet.
// #########################################################
void process_packet(volatile command_packet *pkt)
{


	switch (pkt->command)
	{
		case WRITE_ATTEN:
			parse_atten(pkt);
			break;

		case WRITE_SWITCH:
			parse_switch(pkt);
			break;

		case WRITE_TEMP:
			parse_temp(pkt);
			break;

		case WRITE_MISC:
			parse_misc(pkt);
			break;

		case WRITE_FREQ:
			parse_vfo(pkt);
			break;

		case WRITE_SWEEPER:
			parse_sweeper(pkt);
			break;

		default:
			debugUnique("ERROR! Unknown command!");
			break;
	}


}


// #########################################################
// State machine main parse call.
// called for each received byte.
// #########################################################
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
				debugUnique("ERROR! Not start byte! Wat?");
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

				debugUnique("ERROR! Invalid checksum!");
				debugUnique("Should be  %i, received %i.", checksum, dataByte);
			}
			// Falls through to default (intentionally)!
		default:
			state = NO_PACKET;
			checksum = 0;
			break;
	}

}




// #########################################################
// Random initialization stuff that has to go here because
// you cannot extern a class, and the vfo is global in this
// file
// #########################################################

void setup_vfo(void)
{

	vfo.initialize();
	vfo.setFrequency(100E6);

}

