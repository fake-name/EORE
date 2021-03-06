
#ifndef SI570_H__
#define SI570_H__


#define SI570_I2C_ADDRESS 0x55
typedef enum
{
	SI570_ERROR = 0,
	SI570_READY,
	SI570_SUCCESS
} Si570_Status;


class Si570
{
public:
	Si570(uint8_t i2c_address, uint32_t calibration_frequency);
	Si570_Status setFrequency(uint32_t newfreq);
	void debugSi570();
	void initialize();

	Si570_Status status;

private:
	uint8_t i2c_address;
	uint8_t dco_reg[13];
	uint32_t f_center;
	uint32_t frequency;
	uint16_t hs, n1;
	uint64_t freq_xtal;
	uint64_t fdco;
	uint64_t rfreq;
	uint32_t max_delta;


	bool read_si570();
	void write_si570();
	void qwrite_si570();

	uint8_t getHSDIV();
	uint8_t getN1();
	uint64_t getRFREQ();

	void calculateRFREQRegisters(uint32_t fnew);
	Si570_Status findDivisors(uint32_t f);
};


#endif // SI570_H__
