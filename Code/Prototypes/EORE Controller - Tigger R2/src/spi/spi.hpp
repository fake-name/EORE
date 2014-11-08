/*
 * spi.hpp
 *
 * Created: 11/6/2014 3:35:32 PM
 *  Author: Fake Name
 */


#ifndef SPI_H_
#define SPI_H_



typedef enum
{
	SET_ERROR = -1,
	SET_SUCCESS = 0
} Spi_Status;



Spi_Status writeAttenuator(uint8_t atten, uint8_t val);
Spi_Status writeSwitch(uint8_t swNo, uint8_t val);

void spiWrite(uint8_t data);

#endif /* SPI_H_ */