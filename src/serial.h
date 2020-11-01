#ifdef __cplusplus
extern "C" {
#endif
#ifndef SERIAL_H_
#define SERIAL_H_

#include <string.h>

#define F_CPU 16000000UL // Atmega 328P 16Mhz
#define BAUD 38400UL
#define UBRR ((F_CPU/16/BAUD) - 1)

void uart_init();
void uart_putchar(unsigned char chr);
void uart_putstr(const char *str);
const unsigned char uart_getchar();
void uart_echo(void);
void uart_getline(char* command_buf, int size);
const char serial_available();

#endif
#ifdef __cplusplus
}
#endif