


#include "print/print.hpp"
#include <stdarg.h>
#include <asf.h>
// #include <inttypes.h>
// #include <string.h>


// #########################################################
// Ring Buffer
// #########################################################

#define RX_BUFFER_SIZE 128

struct ring_buffer {
	unsigned char buffer[RX_BUFFER_SIZE];
	int head;
	int tail;
};

ring_buffer rx_buffer = { { 0 }, 0, 0 };

inline void store_char(unsigned char c)
{
	int i = (rx_buffer.head + 1) % RX_BUFFER_SIZE;

	// if we should be storing the received character into the location
	// just before the tail (meaning that the head would advance to the
	// current location of the tail), we're about to overflow the buffer
	// and so we don't write the character or advance the head.
	if (i != rx_buffer.tail) {
		rx_buffer.buffer[rx_buffer.head] = c;
		rx_buffer.head = i;
	}
}

int rxRead(void)
{
	// if the head isn't ahead of the tail, we don't have any characters
	if (rx_buffer.head != rx_buffer.tail) 
	{
		unsigned char c = rx_buffer.buffer[rx_buffer.tail];
		rx_buffer.tail = (rx_buffer.tail + 1) % RX_BUFFER_SIZE;
		return c;
	}
	return -1;
}


int rxAvailable(void)
{
	return (RX_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % RX_BUFFER_SIZE;
}


// #########################################################
// The actual Interrupt
// #########################################################

void DEBUG_UART_ISR_HANDLER(void)
{
	uint32_t dw_status = usart_get_status(DEBUG_UART);
	if (dw_status & US_CSR_RXRDY) 
	{
		uint32_t received_byte;
		usart_read(DEBUG_UART, &received_byte);
		store_char(received_byte);	
	}
}


// #########################################################
// Public functions
// #########################################################

void USARTWriteStr(char const *data)
{
	while( *data != '\0' )
	{
		usart_putchar(DEBUG_UART, *data++);
	}
}


void USARTWriteStrLn(char const *data)
{
	while( *data != '\0' )
	{
		usart_putchar(DEBUG_UART, *data++);
}
	usart_putchar(DEBUG_UART, '\r');
	usart_putchar(DEBUG_UART, '\n');
}


void debugUnique(char const *fmt, ... )
{
		char buf[128]; // resulting string limited to 128 chars
		va_list args;
		va_start (args, fmt );
		vsnprintf(buf, 128, fmt, args);
		va_end (args);
		USARTWriteStrLn(buf);
}
