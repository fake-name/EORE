/*
 * TMP100 Temp-sensor library
 */

#include <inttypes.h>

// #include "A1Main.h"
#include "tmp/tmp100.hpp"
#include "print/print.hpp"
// #include "debug.h"
#include "i2c/i2c.h"
#include <asf.h>

// Private function forward definitions
int sensor_to_address(uint8_t sensorNo);
void set_remote_addr(uint8_t addr);

// #define DEBUG_PR(x ...)  // Default to NO debug
#define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug



/*
Chip specific details.
*/


/*
#####################################################
Interfacing
#####################################################
*/

void setup_all_tmp100(void)
{
	
	for (uint8_t cnt = 0; cnt <= 8; cnt++)
	{
		initialize_tmp100(cnt);
		delay_ms(20);
	}	
}


int sensor_to_address(uint8_t sensorNo)
{
	if (sensorNo < 0)
	{
		return -1;
	}
	if (sensorNo > 7)
	{
		return -1;
	}
	return SI570_BASE_I2C_ADDRESS + sensorNo;

}


TMP100_Status initialize_tmp100(uint8_t sensorNo)
{

	set_i2c_speed(400000); // 400 Khz I2C Speed mode
	
	int address = sensor_to_address(sensorNo);
	if (address < 0)
	{
		return TMP100_ERROR;
	}

	// Force Si570 to reset to initial freq
	DEBUG_PR("Setting up TMP sensor at addess %i", address);

	int ret;

	ret = i2c_write(address, TMP100_CONFIG_REGISTER, 0x60);  // High resolution mode, all other functions turned off

	DEBUG_PR("Set up TMP sensor at addess %i. Return value: %i", address, ret);
	if (ret == 0)
	{
		return TMP100_SUCCESS;
	}

	return TMP100_ERROR;


}


TMP100_Status read_temp(uint8_t sensorNo, uint16_t *tempValue)
{
	
	set_i2c_speed(400000); // 400 Khz I2C Speed mode
	
	int address = sensor_to_address(sensorNo);
	if (address < 0)
	{
		return TMP100_ERROR;
	}



	int ret;
	
	// Probe the address of the temperature sensor, so we can be confident it's there, and 
	// we therefor don't get stuck in a read-lock when reading from a non-existent
	// device.
	ret = twi_probe(TWI0, address);
	if (ret != TWI_SUCCESS)
	{
		return TMP100_ERROR;
	}
	
	ret = i2c_read(address, TMP100_TEMPERATURE_REGISTER, (uint8_t*) tempValue, 2);

	// Reverse byte-ordering because the sensor returns LSBfirst, and we need MSBfirst
	*tempValue = ((uint16_t)0xFF00 & *tempValue) >> 8 | ((uint16_t)0x00FF & *tempValue) << 8;

	DEBUG_PR("Read bytes: %X, %X ", ((uint8_t*) tempValue)[0], ((uint8_t*) tempValue)[1]);


	if (ret != 0)
	{
		return TMP100_SUCCESS;
	}
	return TMP100_ERROR;



}


