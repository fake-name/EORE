

#include <inttypes.h>
#include "print/print.hpp"
#include "i2c/i2c.h"

#include <asf.h>


#define DEBUG_PR(x ...)  // Default to NO debug
// #define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug


/*
#####################################################
I2C Calls
#####################################################
*/


// Write a byte to I2C device
void i2c_write(uint8_t i2c_address, uint8_t reg_address, uint8_t data)
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
	packet.chip = (uint32_t) i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
	/* Internal chip address */
	packet.addr[0] = reg_address;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_write(TWI0, &packet);


	if (status != TWI_SUCCESS)
	{
		DEBUG_PR("Error writing %i byte to register %i: %i", 1, reg_address, status);
	}
}

// Write length bytes to I2C device.
int i2c_write(uint8_t i2c_address, uint8_t reg_address, uint8_t *data, uint8_t length)
{
	// Wire.beginTransmission(i2c_address);
	// Wire.write(reg_address);
	// Wire.write(data, length);

	// int error = Wire.endTransmission();
	// if (error != 0)
	// {
	// 	DEBUG_PR("Error writing %i bytes to register %i: %i", length, reg_address, error);
	// 	return -1;
	// }

	twi_packet_t packet;
	volatile uint32_t status;

	/* Data to send */
	packet.buffer = data;
	/* Data length */
	packet.length = length;
	/* Slave chip address */
	packet.chip = (uint32_t) i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
	/* Internal chip address */
	packet.addr[0] = reg_address;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_write(TWI0, &packet);

	if (status != TWI_SUCCESS)
	{
		DEBUG_PR("Error writing %i bytes to register %i: %i", length, reg_address, status);
		return -1;
	}
	return length;
}

// Read a one byte register from the I2C device
uint8_t i2c_read(uint8_t i2c_address, uint8_t reg_address)
{
	uint8_t rdata = 0xFF;
	// Wire.beginTransmission(i2c_address);
	// Wire.write(reg_address);
	// Wire.beginTransmission(i2c_address);
	// Wire.write(reg_address);
	// Wire.endTransmission();
	// Wire.requestFrom(i2c_address, (uint8_t)1);
	// if (Wire.available()) rdata = Wire.read();


	twi_packet_t packet;
	volatile uint32_t status;

	/* Data to send */
	packet.buffer = &rdata;
	/* Data length */
	packet.length = 1;
	/* Slave chip address */
	packet.chip = (uint32_t) i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
	/* Internal chip address */
	packet.addr[0] = reg_address;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_read(TWI0, &packet);

	if (status != TWI_SUCCESS)
	{
		DEBUG_PR("Error reading %i byte from register %i: %i", 1, reg_address, status);

	}

	return rdata;
}

// Read multiple bytes fromt he I2C device
int i2c_read(uint8_t i2c_address, uint8_t reg_address, uint8_t *output, uint8_t length)
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
	volatile uint32_t status;

	/* Data to send */
	packet.buffer = output;
	/* Data length */
	packet.length = length;
	/* Slave chip address */
	packet.chip = (uint32_t) i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
	/* Internal chip address */
	packet.addr[0] = reg_address;
	packet.addr[1] = 0;
	packet.addr[2] = 0;
	/* Address length */
	packet.addr_length = 1;

	/* Perform a master write access */
	status = twi_master_read(TWI0, &packet);

	if (status != TWI_SUCCESS)
	{
		DEBUG_PR("Error reading %i bytes from register %i: %i", length, reg_address, status);
		return 0;
	}

	return length;
}
/*
 * i2c.cpp
 *
 * Created: 1/21/2015 6:59:50 PM
 *  Author: Fake Name
 */
