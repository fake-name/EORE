/*
 * Si570 Library for Arduino
 *
 * MIT License
 *
 * Copyright Jeff Whitlatch - ko7m - 2014
 * Based on previous work by Thomas Sarlandie which was
 * based on previous work by Ashar Farhan
 */

#include <inttypes.h>

// #include "A1Main.h"
#include "si570/Si570.hpp"
#include "print/print.hpp"
// #include "debug.h"

#include <asf.h>



#define DEBUG_PR(x ...)  // Default to NO debug
// #define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug


// Initialize the Si570 and determine its internal crystal frequency given the default output frequency
Si570::Si570(uint8_t si570_address, uint32_t calibration_frequency)
{

	i2c_address = si570_address;
	// DEBUG_PR("Si570 init, calibration frequency = %lu", calibration_frequency);
	// Wire.begin();

	// Disable internal pullups - You will need external 3.3v pullups.
	// digitalWrite(SDA, 0);
	// digitalWrite(SCL, 0);

	// We are about the reset the Si570, so set the current and center frequency to the calibration frequency.
	this->f_center = calibration_frequency;
	this->frequency = calibration_frequency;
}

void Si570::initialize()
{
	max_delta = ((uint64_t) f_center * 10035LL / 10000LL) - f_center;

	// Force Si570 to reset to initial freq
	DEBUG_PR("Resetting Si570");
	i2c_write(135,0x01);
	delay_ms(20);

	if (read_si570())
	{
		DEBUG_PR("Successfully initialized Si570");
		freq_xtal = (unsigned long) ((uint64_t) this->frequency * getHSDIV() * getN1() * (1L << 28) / getRFREQ());
		status = SI570_READY;
	}
	else
	{
		// Use the factory default if we were unable to talk to the chip
		freq_xtal = 114285000L;
		DEBUG_PR("Unable to properly initialize Si570");
		status = SI570_ERROR;
	}

	DEBUG_PR("freq_xtal = %04lu", freq_xtal);
}



/*
#####################################################
I2C Calls
#####################################################
*/


// Write a byte to I2C device
void Si570::i2c_write(uint8_t reg_address, uint8_t data)
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
	packet.chip = (uint32_t) this->i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
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
int Si570::i2c_write(uint8_t reg_address, uint8_t *data, uint8_t length)
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
	packet.chip = (uint32_t) this->i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
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
uint8_t Si570::i2c_read(uint8_t reg_address)
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
	packet.chip = (uint32_t) this->i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
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
int Si570::i2c_read(uint8_t reg_address, uint8_t *output, uint8_t length)
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
	packet.chip = (uint32_t) this->i2c_address;  // Device address must be >> by one, since apparently the driver injects the read/write flag internally
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
Chip specific details.
*/




/*
#####################################################
Interfacing
#####################################################
*/





// Debug routine for examination of Si570 state
void Si570::debugSi570()
{

	DEBUG_PR(" --- Si570 Debug Info ---");

	DEBUG_PR("Status: %i", status);

	for (int i = 7; i < 15; i++) {
		DEBUG_PR("Register[%i] = %02x", i, dco_reg[i]);
	}

	DEBUG_PR("HSDIV = %i, N1 = %i", getHSDIV(), getN1());

	DEBUG_PR("Xtal Freq = %llu", freq_xtal);

	DEBUG_PR("RFREQ (hex): %04lx%04lx", (uint32_t)((getRFREQ() >> 32)), (uint32_t)(getRFREQ() & 0xffffffff));
}

// Return the 8 bit HSDIV value from register 7
uint8_t Si570::getHSDIV()
{
	uint8_t hs_reg_value = dco_reg[7] >> 5;
	return 4 + hs_reg_value;
}

// Compute and return the 8 bit N1 value from registers 7 and 8
uint8_t Si570::getN1()
{
	uint8_t n_reg_value = ((dco_reg[7] & 0x1F) << 2) + (dco_reg[8] >> 6);
	return n_reg_value + 1;
}

// Return 38 bit RFREQ value in a 64 bit integer
uint64_t Si570::getRFREQ()
{
	fdco  = (uint64_t)(dco_reg[8] & 0x3F) << 32;
	fdco |= (uint64_t) dco_reg[9] << 24;
	fdco |= (uint64_t) dco_reg[10] << 16;
	fdco |= (uint64_t) dco_reg[11] << 8;
	fdco |= (uint64_t) dco_reg[12];

	return fdco;
}


// Read the Si570 chip and populate dco_reg values
bool Si570::read_si570(){
	// Try 3 times to read the registers
	for (int i = 0; i < 3; i++)
	{
		// we have to read eight consecutive registers starting at register 7
		if (i2c_read(7, &(dco_reg[7]), 6) == 6)
		{
			return true;
		}
		DEBUG_PR("Error reading Si570 registers... Retrying.");
		delay_ms(50);
	}
	return false;
}

// Write dco_reg values to the Si570
void Si570::write_si570()
{
	int idco;

	// Freeze DCO
	idco = i2c_read(137);
	i2c_write(137, idco | 0x10 );

	i2c_write(7, &dco_reg[7], 6);

	// Unfreeze DCO
	i2c_write(137, idco & 0xEF);

	// Set new freq
	i2c_write(135,0x40);
}

// In the case of a frequency change < 3500 ppm, only RFREQ must change
void Si570::qwrite_si570()
{
	int idco;

	// Freeze the M Control Word to prevent interim frequency changes when writing RFREQ registers.
	idco = i2c_read(135);
	i2c_write(135, idco | 0x20);

	// Write RFREQ registers
	i2c_write(7, &dco_reg[7], 6);

	// Unfreeze the M Control Word
	i2c_write(135, idco &  0xdf);
}

#define fDCOMinkHz 4850000	// Minimum DCO frequency in kHz
#define fDCOMaxkHz 5670000  // Maximum DCO frequency in KHz

// Locate an appropriate set of divisors (HSDiv and N1) give a desired output frequency
Si570_Status Si570::findDivisors(uint32_t fout)
{
	const uint16_t HS_DIV[] = {11, 9, 7, 6, 5, 4};
	uint32_t fout_kHz = fout / 1000;

	// Floor of the division
	uint16_t maxDivider = fDCOMaxkHz / fout_kHz;

	// Ceiling of the division
	n1 = 1 + ((fDCOMinkHz - 1) / fout_kHz / 11);

	if (n1 < 1 || n1 > 128)
		return SI570_ERROR;

	while (n1 <= 128)
	{
		if (0 == n1 % 2 || 1 == n1)
		{
			// Try each divisor from largest to smallest order to minimize power
			for (int i = 0; i < 6 ; ++i)
			{
				hs = HS_DIV[i];
				if (hs * n1 <= maxDivider)
					return SI570_SUCCESS;
			}
		}
		n1++;
	}
	return SI570_ERROR;
}

// Set RFREQ register (38 bits)
void Si570::calculateRFREQRegisters(uint32_t fnew)
{
	// Calculate new DCO frequency
	fdco = (uint64_t) fnew * hs * n1;

	// Calculate the new RFREQ value
	rfreq = (fdco << 28) / freq_xtal;

	// Round the result
	// rfreq = rfreq + ((rfreq & 1<<(28-1))<<1);

	// Reset all Si570 registers
	for (int i = 7; i <= 12; i++)
		dco_reg[i] = 0;

	// Set up the RFREQ register values
	dco_reg[12] = rfreq & 0xff;
	dco_reg[11] = rfreq >> 8 & 0xff;
	dco_reg[10] = rfreq >> 16 & 0xff;
	dco_reg[9]  = rfreq >> 24 & 0xff;
	dco_reg[8]  = rfreq >> 32 & 0x3f;

	// set up HS and N1 in registers 7 and 8
	dco_reg[7]  = (hs - 4) << 5;
	dco_reg[7]  = dco_reg[7] | ((n1 - 1) >> 2);
	dco_reg[8] |= ((n1-1) & 0x3) << 6;

	debugSi570();
}

// Set the Si570 frequency
Si570_Status Si570::setFrequency(uint32_t newfreq)
{
	Si570_Status status;
	// If the current frequency has not changed, we are done
	if (frequency == newfreq)
	{
		return SI570_SUCCESS;
	}

	// TODO: Status return for this function is currently garbage. Fix?

	// otherwise it is a big jump and we need a new set of divisors and reset center frequency
	status = findDivisors(newfreq);
	if (status == SI570_ERROR)
	{
		return status;
	}
	
	calculateRFREQRegisters(newfreq); // No return value
	
	write_si570();

	DEBUG_PR("RFREQ (dec): %lu", newfreq); // DEBUG_PR ###
	return SI570_SUCCESS;
}