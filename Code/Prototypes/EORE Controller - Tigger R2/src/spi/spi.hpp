/*
 * spi.hpp
 *
 * Created: 11/6/2014 3:35:32 PM
 *  Author: Fake Name
 */


#ifndef SPI_H_
#define SPI_H_


void writeAttenuator(uint8_t atten, uint8_t val);
void writeSwitch(uint8_t swNo, uint8_t val);

void spiWrite(uint8_t data);

#endif /* SPI_H_ */