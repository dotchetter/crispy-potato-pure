#include <avr/io.h>
#include "serial.h"


void uart_init(unsigned int ubrr)
{
    // Left shift the UBRR0H Register by ubrr 8 positions
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Enable receiver and transmitter registers
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);

    // Set the frame format, being 8N1
    UCSR0C = (1<<USBS0) | (3<<UCSZ00);
}

void uart_putchar(unsigned char chr)
{
    /* 
    Suspend and await equal state between 
    registers to ensure empty transmit buffer 
    */
    while(!(UCSR0A & (1<<UDRE0)))
    ;

    UDR0 = chr;
}

void uart_putstr(const char *str)
{
    while (*str)
    {
        uart_putchar(*str);
        str++;
    }
}

char uart_getchar(void)
{
    // Await data to be received
    while (!(UCSR0A & (1<<RXC0)))
    ;
    return UDR0;
}

//void uart_echo(void);