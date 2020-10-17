#ifndef SERIAL_H_
#define SERIAL_H_

#define F_OSC 16000000UL // Atmega 328P 16Mhz
#define BAUD 115200UL
#define MYUBRR (((F_OSC / (BAUD * 16UL))) -1)

void uart_init(unsigned int ubrr);
void uart_putchar(unsigned char chr);
void uart_putstr(const char *str);
char uart_getchar(void);
void uart_echo(void);

#endif