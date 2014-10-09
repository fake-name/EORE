

// Definition of interrupt names
#include < avr/io.h >
// ISR interrupt service routine
#include < avr/interrupt.h >

// Arduino crap
#include "Arduino.h"

#define F_CPU 16000000UL  // 1 MHz
#include <util/delay.h>

typedef enum
{
	NO_PACKET,
	HAVE_HEADER,
	HAVE_COMMAND,
	HAVE_DATA,
	PACKET_VALID
} sysState;

typedef struct
{
	uint8_t command;
	uint8_t value;
} command_packet;

#define PACKET_HEADER 0x5D
#define READ_CMD  0x25
#define WRITE_CMD 0xAA

volatile uint8_t checksum = 0;

volatile command_packet packet = {0, 0};
volatile sysState state = NO_PACKET;


// Forward definitions
void parse(uint8_t dataByte);
void process_packet(sysState *pkt);


void process_packet(volatile command_packet *pkt)
{
	if (pkt->command == READ_CMD)
	{
		Serial.print("READ ");
		Serial.println(PORTC);

	}
	else if (pkt->command == WRITE_CMD)
	{
		Serial.print("WRITE ");
		Serial.println(pkt->value);
		PORTC = pkt->value;
	}
	else
	{
		Serial.println("Error! Unknown command!");
	}
}

void parse(uint8_t dataByte)
{
	switch (state)
	{
		case NO_PACKET:
			if (dataByte == PACKET_HEADER)
			{
				state = HAVE_HEADER;
				checksum += dataByte;
			}
			break;

		case HAVE_HEADER:
			packet.command = dataByte;
			state = HAVE_COMMAND;
			checksum += dataByte;
			break;

		case HAVE_COMMAND:

			packet.value = dataByte;
			state = HAVE_DATA;
			checksum += dataByte;
			break;

		case HAVE_DATA:

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

int main()
{

	Serial.begin(115200);

	// enable interrupts
	sei();

	DDRC = 0xFF;

	while(1)
	{
		if (Serial.available())
		{
			uint8_t ser = Serial.read();
			parse(ser);
		}
	}

}