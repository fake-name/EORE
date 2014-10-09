
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
#define WRITE_SWITCH  0xC0


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
			Serial.println(pkt->value);
			writeAtten(0, pkt->value);
			break;

		case WRITE_ATTEN_2:
			Serial.print("Atten Set 2 ");
			Serial.println(pkt->value);
			writeAtten(1, pkt->value);
			break;

		case WRITE_ATTEN_3:
			Serial.print("Atten Set 3 ");
			Serial.println(pkt->value);
			writeAtten(2, pkt->value);
			break;


		case WRITE_SWITCH:
			Serial.print("Switch Set ");
			Serial.println(pkt->value);
			writeSwitch(pkt->value);
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



void setup(){}


void loop()
{

	for (uint8_t x = 0; x < sizeof(pins); x += 1)
	{
		pinMode(pins[x], OUTPUT);
		digitalWrite(pins[x], 0);
	}

	Serial.begin(115200);


	while(1)
	{
		if (Serial.available())
		{
			uint8_t ser = Serial.read();
			parse(ser);
		}
	}

}