#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "helpers.h"
#include "timer.h"
#include "StateMachine.h"
#include "entity.h"
#include "led.h"


//Pin definitions using CrispyPotato Shield:
#define LED_PWM   6  // (PORTD BIT 6, pin 6)
#define KEY_1  2  // (PORTD BIT 2, pin 2)


// Compiler contract - methods defined below
void idle_state();
void active_state();
void inactive_state();

// -------------------------------------
// --- Globally accessible instances --- 
// -------------------------------------
//
// Statemachine used to bind function pointers to enum.
// For info on this library, see: 
// https://github.com/dotchetter/StateMachineEmbedded

StateMachine<state> stateMachine = StateMachine<state>(IDLE, &idle_state);


// Instantiate entities such as LEDs and Buttons
ENTITY pwm_led;
ENTITY key_1;


// Volatile byte used for interrupts
volatile uint8_t UART_INTERRUPT_TRIGGERED;


void init()
/*
* Initialize the runtime-environment on the MCU. Steps are as defined:
*
* # 0: Set DDRB to listen to all pins except bit 1, 2, 3. (0 0 0 0 1 1 1 0)
* # 1: Enable pin 6 as output for PWM LED
* # 2: Enable pin 2 as input for pushbutton
* # 3: Enable interrupt over USART receive (USART_RX, ATmega328p datasheet 12.1)
* # 4: Initialize UART. Used to receive commands
* # 5: Add states to the statemachine
* # 6: Configure entities
* # 7: Temporarily disable interrupt routines and enable millis
*/
{   
    DDRB |= (_BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3));              //#0
    DDRD |= _BV(PORTD6);                                            //#1
    DDRD &= ~(_BV(PORTD2));                                         //#2
    UCSR0B |= (1 << RXCIE0);                                        //#3

    uart_init();                                                    //#4

    stateMachine.addState(ACTIVE, &active_state);                   //#5
    stateMachine.addState(INACTIVE, &inactive_state);
    stateMachine.setStaticState(INACTIVE);

    pwm_led.registry_bit = LED_PWM;                                 //#6
    pwm_led.is_active = 0;
    pwm_led.port = &OCR0A;

    key_1.registry_bit = KEY_1;
    key_1.port = &PIND;
    key_1.last_updated_ms = 0;
    key_1.debounce_ms = 80;
    key_1.long_press_trigger_ms = 200;

    cli();                                                          //#7
    timer2_init(16000000UL);
    sei();
}


ISR (USART_RX_vect)
/*
* interrupt service routine, triggered by received chars
* in the USART buffer on UDR0 register.
*/
{
    UART_INTERRUPT_TRIGGERED = 1;
}


// --- State functions --- //
void idle_state()
{
    static uint8_t uart_desired_state;
    static unsigned long last_millis = 0;
    state last_state = stateMachine.getStaticState();

    if (debounceKey(&key_1) && keyClicked(&key_1))
    {
        stateMachine.setStaticState(ACTIVE);
    }
    else
    {
        stateMachine.setStaticState(INACTIVE);
    }

    if (last_state != stateMachine.getStaticState())
    {
        stateMachine.transitionTo(stateMachine.getStaticState());
    }
}


void active_state()
{
    uart_putstr("Button pressed");
    stateMachine.release();
}


void inactive_state()
{
    uart_putstr("Button released");
    stateMachine.release();
}


int main()
{
    uint32_t last_millis;
    init();            

    while(1)
    {  
       stateMachine.next()();
    }
}