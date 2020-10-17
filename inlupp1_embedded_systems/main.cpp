#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "serial.h"
#include "StateMachine.h"

// State function pointer declarations
void idle();
void switchLed();


StateMachine<char> sm = StateMachine<char>(0, &idle);


void setup()
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
    sm.addState(1, switchLed);
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

    switch (next_direction)
    {
        case 0: toggleLedOn(3); next_direction = 1; break;
        case 1: toggleLedOff(3); next_direction = 0; break;
    }

    _delay_ms(1000);
    sm.release();
}


int main ()
{
    setup();

    while(1)
    {
        sm.next()();
    }
    return 0;
}








// int main()
// {
//     uart_init(MYUBRR);
//    
//     while(1)
//     {
//         uart_putstr("Testar =)\r\n");
//         _delay_ms(1000);
//     }
// }