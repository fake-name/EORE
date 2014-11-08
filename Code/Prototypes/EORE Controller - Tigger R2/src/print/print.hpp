
#ifndef PRINT_H__
#define PRINT_H__

void USARTWriteStr(char const *data);
void USARTWriteStrLn(char const *data);
void debugUnique(char const *fmt, ... );


int rxRead(void);
int rxAvailable(void);

#endif // PRINT_H__