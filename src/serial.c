#include <avr/io.h>
#include <string.h>
#include "serial.h"

void uart_init()
/*
 * Sets baudrate in UBRR HIGH and LOW registers.
 * :param ubrr: calculated Uart Baud Register Rate (UBRR)
*/
{
    // Set UBRR HIGH and LOW register values, considering provided UBRR value
    UBRR0L = (uint8_t)(UBRR & 0xFF);
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
}

void uart_putchar(unsigned char c)
/*
* Transfer one byte over UART.
*/
{
    while(!serial_write_ready());
    UDR0 = c;
}

void uart_putstr(const char *str)
/*
* Iterate over the pointer until null terminator,
* blocking wait until buffer is ready for write.
*/
{
    while (*str > 0) {
        while (!serial_write_ready()); 
        uart_putchar(*str++);
    }
}

unsigned char uart_getchar()
/*
* Returns byte available in UDR0 register
* at time of call. If the bit is LOW for UDRE0,
* -1 is returned to allow for non-blocking waiting.
*/
{

    if (!serial_available())
        return -1;
    return UDR0;
}

void uart_getline(char* buf)
{
    char c;
    static unsigned long bufcount = 0;

    if (!serial_available())
        return;

    while (c != '\r') {
        c = uart_getchar();
        if (c >= 1)
            buf[bufcount++] = c;

        if (bufcount == (sizeof(buf) / sizeof(buf[0]) - 1))
            break;
    }
    buf[bufcount+1] = '\0';
    bufcount = 0;
}

void uart_echo()
{
    char input[255];
    memset(input, 0, sizeof(input) / sizeof(input[0]));
    uart_getline(input);
    uart_putstr(input);
}