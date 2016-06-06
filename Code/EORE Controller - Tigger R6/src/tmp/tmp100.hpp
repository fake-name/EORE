
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

void setup_all_tmp100(void);
TMP100_Status initialize_tmp100(uint8_t sensorNo);
TMP100_Status read_temp(uint8_t sensorNo, uint16_t *tempValue);


#endif // TMP100_H__
