/*
 * i2c.h
 *
 * Created: 1/21/2015 6:59:59 PM
 *  Author: Fake Name
 */


#ifndef I2C_H_
#define I2C_H_


#define SDA 20
#define SCL 21


uint8_t i2c_read(uint8_t i2c_address, uint8_t reg_address);
int i2c_read(uint8_t i2c_address, uint8_t reg_address, uint8_t *output, uint8_t length);

void i2c_write(uint8_t i2c_address, uint8_t reg_address, uint8_t data);
int i2c_write(uint8_t i2c_address, uint8_t reg_address, uint8_t *data, uint8_t length);



#endif /* I2C_H_ */