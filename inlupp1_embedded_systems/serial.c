#include <avr/io.h>
#include "serial.h"

void uart_init()
/*
 * Sets baudrate in UBRR HIGH and LOW registers.
 * :param ubrr: calculated Uart Baud Register Rate (UBRR)
*/
{
    // Set UBRR HIGH and LOW register values, considering provided UBRR value
    UBRR0L = (uint8_t)(UBRR & 255);
    UBRR0H = (uint8_t)(UBRR >> 8);

    // enable the transmitter and receiver
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

const char serial_write_ready()
/* 
* If UDRE0 is one, the buffer is empty, 
* and therefore  ready to be written. 
* To isolate this, UDRE0 can be masked with UCSR0A. 
* The buffer is empty and ready to be written to when the bit is 1.
*/
{
    return (UCSR0A & (1 << UDRE0));
}

const char serial_available()
/*
* If the RXC0 bit is set to 1, there is data 
* pending to be received over UART, in which 
* case this method returns 1.
*/
{
    return (UCSR0A & (1 << RXC0));
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