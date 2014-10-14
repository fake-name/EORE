

// Definition of interrupt names
#include < avr/io.h >
// ISR interrupt service routine
#include < avr/interrupt.h >

#define SI570_DEBUG

#include <Wire.h>
#include "Si570.h"

Si570 *vfo;
// Arduino crap
#include "Arduino.h"


#define PACKET_HEADER 0x5D
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
			state = NO_PACKET;
			checksum = 0;
			break;
	}

}


void process_packet(volatile command_packet *pkt)
{

	switch (pkt->command)
	{
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





void setup()
{
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

	while (1)

	{
		if (Serial.available())
		{
			uint8_t ser = Serial.read();
			parse(ser);
		}
		// for (long x = 25000000; x < 500000000; x += 1000000)
		// {
		// 	Serial.print("Tuning to ");
		// 	Serial.println(x);

		// 	vfo->setFrequency(x);
		// 	delay(50);

		// }


		// vfo->setFrequency(99.5E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(99.75E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100.25E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100.5E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100.25E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100E6);
		// delay(DELAY_TIME);
		// vfo->setFrequency(99.75E6);
		// delay(DELAY_TIME);

	}
}


void loop() {}  // Fuck yo loop
