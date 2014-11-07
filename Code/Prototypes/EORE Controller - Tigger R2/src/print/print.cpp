


#include "print/print.hpp"
#include <stdarg.h>
#include <asf.h>
// #include <inttypes.h>
// #include <string.h>

void DEBUG_UART_ISR_HANDLER(void)
{
	uint32_t dw_status = usart_get_status(DEBUG_UART);
	if (dw_status & US_CSR_RXRDY) {
		uint32_t received_byte;
		usart_read(DEBUG_UART, &received_byte);
		usart_write(DEBUG_UART, received_byte);
	}
}


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
