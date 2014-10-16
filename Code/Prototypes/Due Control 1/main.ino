
#define SI570_DEBUG

#include <Wire.h>
#include <stdlib.h>
#include "Si570.h"

Si570 *vfo;


#define SW_1 		44
#define SW_2 		30
#define SW_3 		32

#define DATA 		34
#define LATCH_3 	36
#define CLK 		38
#define LATCH_2 	40
#define LATCH_1 	42

uint8_t pins[] = {30, 32, 34, 36, 38, 40, 42, 44};
uint8_t cnt;



#define PACKET_HEADER 0x5D
#define WRITE_ATTEN_1 0xAA
#define WRITE_ATTEN_2 0xAB
#define WRITE_ATTEN_3 0xAC
#define WRITE_SWITCH  0x0C
#define WRITE_FREQ    0xC0


typedef enum
{
	NO_PACKET,
	WAIT_COMMAND,

	WAIT_DATA_1,
	WAIT_DATA_2,
	WAIT_DATA_3,
	WAIT_DATA_4,
	WAIT_CHECKSUM,

	PACKET_VALID
} sysState;




typedef union UnionU32_t
{
	uint8_t  bytes[4];
	uint32_t value;
} unionU32_t;

typedef struct
{
	uint8_t command;
	unionU32_t value;
} command_packet;


volatile uint8_t checksum = 0;

volatile command_packet packet = {0, 0};
volatile sysState state = NO_PACKET;


#define _BV(bit) (1 << (bit))

void writeAtten(uint8_t which, uint8_t atten)
{
	uint8_t cs_pin;
	switch (which)
	{
		case 0:
			cs_pin = LATCH_1;
			break;
		case 1:
			cs_pin = LATCH_2;
			break;
		case 2:
			cs_pin = LATCH_3;
			break;
		default:
			return;
	}

	if (atten >= 64)
	{
		return;
	}

	for (uint8_t i = 0; i < 6; i += 1)
	{
		if (atten & _BV(5-i))
			digitalWrite(DATA, 1);
		else
			digitalWrite(DATA, 0);
		delayMicroseconds(2);
		digitalWrite(CLK, 1);
		delayMicroseconds(2);
		digitalWrite(CLK, 0);
		delayMicroseconds(2);


	}

	digitalWrite(DATA, 0);
	digitalWrite(cs_pin, 1);
	delayMicroseconds(2);
	digitalWrite(cs_pin, 0);



}

void writeSwitch(uint8_t val)
{
	// SO input 1 of the switch is the *MOST* significant bit. Wat?
	if (val & _BV(0))
		digitalWrite(SW_3, 1);
	else
		digitalWrite(SW_3, 0);

	if (val & _BV(1))
		digitalWrite(SW_2, 1);
	else
		digitalWrite(SW_2, 0);

	if (val & _BV(2))
		digitalWrite(SW_1, 1);
	else
		digitalWrite(SW_1, 0);

}







void process_packet(volatile command_packet *pkt)
{

	switch (pkt->command)
	{
		case WRITE_ATTEN_1:
			Serial.print("Atten Set 1 ");
			Serial.println(pkt->value.value);
			writeAtten(0, (uint8_t) pkt->value.value);
			break;

		case WRITE_ATTEN_2:
			Serial.print("Atten Set 2 ");
			Serial.println(pkt->value.value);
			writeAtten(1, (uint8_t) pkt->value.value);
			break;

		case WRITE_ATTEN_3:
			Serial.print("Atten Set 3 ");
			Serial.println(pkt->value.value);
			writeAtten(2, (uint8_t) pkt->value.value);
			break;


		case WRITE_SWITCH:
			Serial.print("Switch Set ");
			Serial.println(pkt->value.value);
			writeSwitch((uint8_t) pkt->value.value);
			break;

		case WRITE_FREQ:
			if (pkt->value.value < 10e6 | pkt->value.value > 810e6 )
			{
				Serial.print("ERROR: Invalid Frequency: ");
				Serial.println(pkt->value.value);
			}
			else
			{
				vfo->setFrequency(pkt->value.value);
				Serial.print("OK: Freq Set: ");
				Serial.println(pkt->value.value);
			}
			break;

		default:
			Serial.println("Error! Unknown command!");
			break;
	}


}


void parse(uint8_t dataByte)
{
	switch (state)
	{
		case NO_PACKET:
			if (dataByte == PACKET_HEADER)
			{
				state = WAIT_COMMAND;
				checksum += dataByte;
			}
			break;

		case WAIT_COMMAND:
			packet.command = dataByte;
			state = WAIT_DATA_1;
			checksum += dataByte;
			break;

		case WAIT_DATA_1:

			packet.value.bytes[0] = dataByte;
			state = WAIT_DATA_2;
			checksum += dataByte;
			break;
		case WAIT_DATA_2:

			packet.value.bytes[1] = dataByte;
			state = WAIT_DATA_3;
			checksum += dataByte;
			break;
		case WAIT_DATA_3:

			packet.value.bytes[2] = dataByte;
			state = WAIT_DATA_4;
			checksum += dataByte;
			break;
		case WAIT_DATA_4:

			packet.value.bytes[3] = dataByte;
			state = WAIT_CHECKSUM;
			checksum += dataByte;
			break;

		case WAIT_CHECKSUM:

			if (dataByte ==  checksum)
			{
				state = PACKET_VALID;
				process_packet(&packet);
			}
			else
			{
				Serial.println("Invalid checksum!");
				Serial.print("Should be ");
				Serial.print(checksum);
				Serial.print(" received ");
				Serial.println(dataByte);
			}
			// Falls through to default (intentionally)!
		default:
			Serial.print("Rx Complete");
			state = NO_PACKET;
			checksum = 0;
			break;
	}

}




void setup(){}


void loop()
{

	for (uint8_t x = 0; x < sizeof(pins); x += 1)
	{
		pinMode(pins[x], OUTPUT);
		digitalWrite(pins[x], 0);
	}

	Serial.begin(115200);


	Serial.println("Device init...");
	vfo = new Si570(SI570_I2C_ADDRESS, 56320000);

	if (vfo->status == SI570_ERROR) {
		// The Si570 is unreachable. Show an error for 3 seconds and continue.
		Serial.println("Si570 comm error");
		delay(3000);
	}


	// This will print some debugging info to the serial console.
	vfo->debugSi570();


	#define DELAY_TIME 200



	vfo->setFrequency(100E6);


	while(1)
	{
		if (Serial.available())
		{
			uint8_t ser = Serial.read();
			parse(ser);
		}
	}

}