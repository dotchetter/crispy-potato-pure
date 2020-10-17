#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "serial.h"
#include "StateMachine.h"

void idle();
StateMachine<char> sm = StateMachine<char>(0, (fp_t)&idle);

void idle()
{
    sm.transitionTo(1);
}

void toggleLedOn(int pin)
{
    PORTB = 1 << pin | PORTB;
    sm.release();
}

void toggleLedOff(int pin)
{
    PORTB = ~(1 << pin) & PORTB;
    sm.release();
}


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
    
    sm.addState(1, (fp_t)&toggleLedOn);
    sm.addState(2, (fp_t)&toggleLedOff);
    sm.setChainedState(1, 2);
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