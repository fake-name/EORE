
#ifndef COMMAND_PARSE_H_
#define COMMAND_PARSE_H_

// #########################################################
// State machine constants
// #########################################################

#define PACKET_HEADER 0x5D
#define WRITE_ATTEN   0xAA
#define WRITE_SWITCH  0x0C
#define WRITE_FREQ    0xC0
#define WRITE_MISC    0xDD

// #########################################################
// State machine typedefs and variables
// #########################################################
typedef enum
{
	NO_PACKET,
	WAIT_COMMAND,
	WAIT_TGT,
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
	uint8_t target;
	unionU32_t value;
} command_packet;


// #########################################################
// State machine Functions
// #########################################################
void parse_misc(volatile command_packet *pkt);
void process_packet(volatile command_packet *pkt);
void parse(uint8_t dataByte);

// And misc stuff
void setup_vfo(void);

#endif /* COMMAND_PARSE_H_ */
