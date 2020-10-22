#include <avr/io.h>
#include <util/atomic.h>
#include <string.h>
#include <util/delay.h>
#include "serial.h"

#define WIN_ENDL '\r'
#define LINUX_ENDL '\n'

void uart_init()
/*
 * Sets baudrate in UBRR HIGH and LOW registers.
 * Referred " UBRR " defined in serial.h
 * 
 * 8N1 is defined by default for the ATmega328p which
 * is why the frame format register is left unchanged.
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
    #ifdef WIN_ENDL
    uart_putchar('\r');
    uart_putchar('\n');
    #else
    uart_putchar('\n');
    #endif
}

const unsigned char uart_getchar()
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

void uart_getline(char* command_buf, int size)
/*
* Read UART until delimiter or buffer exhaustion is reached.
* The function is designed to be called by an interrupt 
* service routine, which is why the loop is declared 
* in an atomic block. The block will run until either the
* UDR0 register returns NULL, a defined delimiter is encountered,
* or the buffer is filled to the fullest.
* 
* For stability reasons a 500 microsecond delay is implemented 
* in each iteration to give the microcontroller a few CPU cycles
* to set the UDR0 register with the next char in sequence. 
* This mitigates an issue where to quick polls from this register
* would yield garbage data and NULL's.
*/
{
    int count = 0;
    char incoming;

    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        while (count < (size-1)) 
        {
            incoming = uart_getchar();

            if (incoming < 1 || incoming == WIN_ENDL || incoming == LINUX_ENDL)
            {
                // Empty the UDR0 register to mitigate leaking '\n' character
                while(serial_available());
                break;
            }

            command_buf[count] = incoming;
            count++;
            _delay_us(500);
        }
        command_buf[count] = '\0';
    }
}

void uart_echo()
{
    char input[255];
    memset(input, 0, sizeof(input) / sizeof(input[0]));
    uart_getline(input, sizeof(input) / sizeof(input[0]));
    uart_putstr(input);
}