/*
 * TMP100 Temp-sensor library
 */

#include <inttypes.h>

// #include "A1Main.h"
#include "tmp/tmp100.hpp"
#include "print/print.hpp"
// #include "debug.h"

#include <asf.h>

// Private function forward definitions
int sensor_to_address(uint8_t sensorNo);
void set_remote_addr(uint8_t addr);

// #define DEBUG_PR(x ...)  // Default to NO debug
#define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug



/*
#####################################################
I2C Calls
#####################################################
*/



// Write a byte to I2C device
int i2c_write_addr(uint32_t i2c_addr, uint8_t reg_address, uint8_t data)
{

	// Wire.beginTransmission(i2c_address);
	// Wire.write(reg_address);
	// Wire.write(data);
	// Wire.endTransmission();

	twi_packet_t packet;
	volatile uint8_t status;

	/* Data to send */
	packet.buffer = &data;

	/* Data length */
	packet.length = 1;

	/* Slave chip address */
	packet.chip = (uint32_t) i2c_addr;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally

	/* Internal chip address */
	packet.addr[0] = reg_address;
	packet.addr[1] = 0;
	packet.addr[2] = 0;

	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_write(DEVICE_TWI, &packet);

	if (status != TWI_SUCCESS)
	{
		DEBUG_PR("Error writing %i byte to address %i register %i, value %i", 1, i2c_addr, reg_address, status);
	}
	// DEBUG_PR("Writing %i byte to address %i register %i, value %i.", 1, i2c_addr, reg_address, status);
	// DEBUG_PR("TWI_MMR = %i", DEVICE_TWI->TWI_MMR);
	// DEBUG_PR("TWI_IADR = %i", DEVICE_TWI->TWI_IADR);

	return status;


}


// Read multiple bytes fromt he I2C device
int i2c_read_addr(uint32_t i2c_addr, uint8_t reg_address, uint8_t *output, uint8_t length)
{
	// int len;

	// Wire.beginTransmission(i2c_address);
	// Wire.write(reg_address);

	// int error = Wire.endTransmission();
	// if (error != 0)
	// {
	// 	DEBUG_PR("Error reading %i bytes from register %i.", reg_address);
	// 	DEBUG_PR(" endTransmission() returned %i", error);
	// 	return 0;
	// }

	// len = Wire.requestFrom(i2c_address,length);
	// if (len != length)
	// {
	// 	DEBUG_PR("Requested %i bytes and only got %i bytes", length, len);
	// }
	// for (int i = 0; i < len && Wire.available(); i++)
	// 	output[i] = Wire.read();

	twi_packet_t packet;
	int status;

	/* Data to send */
	packet.buffer = output;
	/* Data length */
	packet.length = length;
	/* Slave chip address */
	packet.chip = (uint32_t) i2c_addr;
	/* Internal chip address */
	packet.addr[0] = reg_address;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_read(DEVICE_TWI, &packet);

	if (status != TWI_SUCCESS)
	{
		DEBUG_PR("Error reading %i bytes from register %i: %i", length, reg_address, status);
		return 0;
	}

	return status;
}



/*
Chip specific details.
*/


/*
#####################################################
Interfacing
#####################################################
*/



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

	int address = sensor_to_address(sensorNo);
	if (address < 0)
	{
		return TMP100_ERROR;
	}

	// Force Si570 to reset to initial freq
	DEBUG_PR("Setting up TMP sensor at addess %i", address);

	int ret;

	ret = i2c_write_addr(address, TMP100_CONFIG_REGISTER, 0x60);  // High resolution mode, all other functions turned off

	DEBUG_PR("Set up TMP sensor at addess %i. Return value: %i", address, ret);
	if (ret == TWI_SUCCESS)
	{
		return TMP100_SUCCESS;
	}

	return TMP100_ERROR;


}


TMP100_Status read_temp(uint8_t sensorNo, uint16_t *tempValue)
{

	int address = sensor_to_address(sensorNo);
	if (address < 0)
	{
		return TMP100_ERROR;
	}



	int ret;
	ret = i2c_read_addr(address, TMP100_TEMPERATURE_REGISTER, (uint8_t*) tempValue, 2);

	// Reverse byte-ordering because the sensor returns LSBfirst, and we need MSBfirst
	*tempValue = ((uint16_t)0xFF00 & *tempValue) >> 8 | ((uint16_t)0x00FF & *tempValue) << 8;

	DEBUG_PR("Read bytes: %X, %X ", ((uint8_t*) tempValue)[0], ((uint8_t*) tempValue)[1]);


	if (ret == TWI_SUCCESS)
	{
		return TMP100_SUCCESS;
	}
	return TMP100_ERROR;



}


