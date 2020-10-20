#include "serial.h"
#include "StateMachine.h"
#include "millis.c"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// State function pointer declarations
void idle();
void switchLed();

// Globally accessible statemachine instance
StateMachine<char> sm = StateMachine<char>(0, &idle);

void setup()
void init()
/*
    Pin definitions using CrispyPotato Shield:
    
    RED LED:    11 (PORTB BIT 3)
    GREEN LED:  9  (PORTB BIT 1)
    BLUE LED:   10 (PORTB BIT 2)

    BUTTON 1:   8  (PORTB BIT 0)
    BUTTON 2:   12 (PORTB BIT 4)
    DDRB MUST BE: 00001110 (0 TO LISTEN, 1 TO WRITE)
*/
{
    // Set DDRB to listen to all pins except bit 1, 2, 3. (0 0 0 0 1 1 1 0)
    DDRB = (7 << 1);

    // Enable interrupt over USART receive (USART_RX, ATmega328p datasheet 12.1)
    UCSR0B |= (1 << RXCIE0);

    uart_init();

    /*
    * State map: 0 :: idle function, main state
                 1 :: switchLed
                 2 :: parseUart
    */
    sm.addState(1, switchLed);

    // Disable interrupt routines and enable millis
    cli();
    init_millis(F_CPU);

    // Enable interrupt routines
    sei();
}

// Configure interrupt routine for USART_RX vector, with defined action
ISR (USART_RX_vect)
{
    uart_echo();
}

void idle()
{
    sm.transitionTo(1);
}

void toggleLedOn(int led)
{
    PORTB = 1 << led | PORTB;
}

void toggleLedOff(int led)
{
    PORTB = ~(1 << led) & PORTB;
}

void switchLed()
{
    static char next_direction;

    switch (next_direction) {
        case 0: toggleLedOn(3); next_direction = 1; break;
        case 1: toggleLedOff(3); next_direction = 0; break;
    }

   sm.release();
}

int main()
{
    unsigned long last_iter_ms;
    setup();
    init();
    
    while(1)
    {
        if (millis() - last_iter_ms >= 250)
        {
            switchLed();
            last_iter_ms = millis();
        }
        //_delay_ms(500);
        //uart_echo();
    }
}