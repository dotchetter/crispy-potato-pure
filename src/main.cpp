#include "serial.h"
#include "StateMachine.h"
#include "millis.c"
#include "commands.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Pin definitions using CrispyPotato Shield:
#define LED_RED 3       // (PORTB BIT 3)
#define LED_GREEN 9     // (PORTB BIT 1)
#define LED_BLUE 10     // (PORTB BIT 2)
#define BUTTON_1 8      // (PORTB BIT 0)
#define BUTTON_2 12     // (PORTB BIT 4)

// Macros
#define toggle_led_on(LED) (PORTB = 1 << LED | PORTB)
#define toggle_led_off(LED) (PORTB = ~(1 << LED) & PORTB)

// State function pointer declarations
void idle();
void switchLed();

// Globally accessible statemachine instance
StateMachine<char> sm = StateMachine<char>(0, &idle);

// Volatile byte used for interrupts
volatile uint8_t USART_INTERRUPT_TRIGGERED;

void init()
{
    // DDRB MUST BE: 00001110 (0 TO LISTEN, 1 TO WRITE)
    // Set DDRB to listen to all pins except bit 1, 2, 3. (0 0 0 0 1 1 1 0)
    DDRB = (7 << 1);

    // Enable interrupt over USART receive (USART_RX, ATmega328p datasheet 12.1)
    UCSR0B |= (1 << RXCIE0);

    uart_init();

    /* State map: 0 :: idle function, main state
                  1 :: switchLed
                  2 :: parseUart
    */
    sm.addState(1, &switchLed);

    // Disable interrupt routines and enable millis
    cli();
    init_millis(F_CPU);

    // Enable interrupt routines
    sei();
}

// Configure interrupt routine for USART_RX vector, with defined action
ISR (USART_RX_vect)
{
    USART_INTERRUPT_TRIGGERED = 1;
}

void parseUartCommand(char* buf)
{
    uart_getline(buf);
}

void switchLed()
{
    static char next_direction;

    switch (next_direction) {
        case 0: toggle_led_on(LED_RED); next_direction = 1; break;
        case 1: toggle_led_off(LED_RED); next_direction = 0; break;
    }

   sm.release();
}

void idle()
{
    static unsigned long last_iter_ms;
    char command_buf[128];
    
    if (USART_INTERRUPT_TRIGGERED)
    {
        parseUartCommand(command_buf);
        switchLed();
        USART_INTERRUPT_TRIGGERED = 0;
        uart_putstr(command_buf);
    }
    
    if (millis() - last_iter_ms >= 250)
    {
        // TODO
        last_iter_ms = millis();
    }
}

int main()
{
    fp_t next_state;
    
    init();
    
    for(;;)
    {
        next_state = sm.next();
        next_state();
    }
}