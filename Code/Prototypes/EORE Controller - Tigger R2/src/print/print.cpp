


#include "print/print.hpp"
#include <stdarg.h>
#include <asf.h>
// #include <inttypes.h>
// #include <string.h>


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
