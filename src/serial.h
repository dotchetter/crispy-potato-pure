#ifdef __cplusplus
extern "C" {
#endif
#ifndef SERIAL_H_
#define SERIAL_H_


#define F_CPU 16000000 // Atmega 328P 16Mhz
#define BAUD 38400
#define UBRR ((F_CPU/16/BAUD) - 1)

void uart_init();
void uart_putchar(unsigned char chr);
void uart_putstr(const char *str);
unsigned char uart_getchar(void);
void uart_echo(void);
void uart_getline(char* buf);
const char serial_available();

#endif
#ifdef __cplusplus
}
#endif