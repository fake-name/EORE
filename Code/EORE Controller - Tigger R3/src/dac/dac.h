/*
 * dac.h
 *
 * Created: 1/20/2015 6:25:42 PM
 *  Author: Fake Name
 */ 


#ifndef DAC_H_
#define DAC_H_


#define DAC_I2C_ADDRESS 0x0F
#define SDA 20
#define SCL 21

void enableDac(void);
void writeDac(uint8_t channel, uint16_t value);


#endif /* DAC_H_ */