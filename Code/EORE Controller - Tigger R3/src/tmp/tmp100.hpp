
#ifndef TMP100_H__
#define TMP100_H__


#define SI570_BASE_I2C_ADDRESS 0x48


typedef enum
{
	TMP100_ERROR = 0,
	TMP100_SUCCESS
} TMP100_Status;

#define TMP100_CONFIG_REGISTER        1
#define TMP100_TEMPERATURE_REGISTER   0

int i2c_write_addr(uint32_t i2c_addr, uint8_t reg_address, uint8_t data);
// int i2c_write_addr(uint32_t i2c_addr, uint8_t reg_address, uint8_t *data, uint8_t length);
// int i2c_read_addr(uint32_t i2c_addr, uint8_t reg_address, uint8_t *rdata);
int i2c_read_addr(uint32_t i2c_addr, uint8_t reg_address, uint8_t *output, uint8_t length);

TMP100_Status initialize_tmp100(uint8_t sensorNo);
TMP100_Status read_temp(uint8_t sensorNo, uint16_t *tempValue);


#endif // TMP100_H__
