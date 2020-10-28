#include "serial.h"
#include "timer.h"
#include "StateMachine.h"
#include "commands.h"
#include "entity.h"
#include "led.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

//Pin definitions using CrispyPotato Shield:
#define LED_RED   3  // (PORTB BIT 3, pin 11)
#define LED_GREEN 1  // (PORTB BIT 1, pin 9)
#define LED_BLUE  2  // (PORTB BIT 2, pin 10)
#define LED_PWM   6  // (PORTD BIT 6, pin 6)
#define BUTTON_1  0  // (PORTB BIT 0, pin 8)
#define BUTTON_2  4  // (PORTB BIT 4, pin 12)

// Preferences
#define LED_BLINK_INTERVAL_MS 1000

// Compiler contract - methods defined below
void armed_state();
void disarmed_state();
void idle_state();

// -------------------------------------
// --- Globally accessible instances --- 
// -------------------------------------
//
// Statemachine used to bind function pointers to chars.
// For info on this library, see: 
// https://github.com/dotchetter/StateMachineEmbedded

StateMachine<char> sm = StateMachine<char>(0, &idle_state);

ENTITY_LED red_led;
ENTITY_LED green_led;
ENTITY_LED blue_led;
ENTITY_LED pwm_led;

// Volatile byte used for interrupts
volatile uint8_t UART_INTERRUPT_TRIGGERED;

void init()
{
    // (0 TO LISTEN (INPUT), 1 TO WRITE (OUTPUT))
    // Set DDRB to listen to all pins except bit 1, 2, 3. (0 0 0 0 1 1 1 0)
    DDRB |= (_BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3)); 
    
    // Enable pin 6 as output, which isn
    DDRD |= _BV(PORTD6);

    // Enable interrupt over USART receive (USART_RX, ATmega328p datasheet 12.1)
    UCSR0B |= (1 << RXCIE0);

    // Initialize UART. Used to receive commands
    uart_init();
    
    // Add states
    sm.addState(1, &armed_state);
    sm.addState(2, &disarmed_state);

    // Configure LED entities
    red_led.registry_bit = LED_RED;
    red_led.is_lit = 0;

    green_led.registry_bit = LED_GREEN;
    green_led.is_lit = 0;

    blue_led.registry_bit = LED_BLUE;
    blue_led.is_lit = 0;

    pwm_led.registry_bit = LED_PWM;
    pwm_led.is_lit = 0;

    // Temporarily disable interrupt routines and enable millis
    cli();
    timer_init();
}

const uint8_t parse_command()
/*
* Read the incoming command over USART. Parses the command and
  returns the int value for map place of given command in the 
  lookup table for PROGMEM commands, defined in commands.h.
  returns 0 if no command matched.
*/
{
    char input[255];
    char buff[3];

    uart_getline(input, sizeof(input) / sizeof(input[0]));
    
    if (strcasecmp_P(input, command_on) == 0)
    {
        return 1;
    }
    else if(strcasecmp_P(input, command_off) == 0)
    {
        return 2;
    }
    else if(strcasecmp_P(input, command_die) == 0)
    {
        toggle_led_off(&red_led);
        toggle_led_off(&green_led);
    }
    return 0;
}


// Functions representing different states, coupled to the statemachine 

void disarmed_state()
{
    toggle_led_on(&green_led);
    toggle_led_off(&red_led);
}

void armed_state()
{
    static unsigned long last_call_ms;

    toggle_led_off(&green_led);

    if (millis() - last_call_ms >= LED_BLINK_INTERVAL_MS)
    {
        if(red_led.is_lit)
        {
            toggle_led_off(&red_led);
        }
        else
        {
            toggle_led_on(&red_led);
        }
        last_call_ms = millis();
    }    
    sm.release();
}

void idle_state()
{
    static uint8_t uart_desired_state;
    
    // Transition to the state which maps to the char given by the interrupt routine
    if (UART_INTERRUPT_TRIGGERED)
    {
        uart_desired_state = parse_command();
        UART_INTERRUPT_TRIGGERED = 0;
    }    

    sm.transitionTo(uart_desired_state);
}


// Interrupt Service Routine definitions

ISR (USART_RX_vect)
/*
* interrupt service routine, triggered by received chars
* in the USART buffer on UDR0 register.
*/
{
    UART_INTERRUPT_TRIGGERED = 1;
}



int main()
{
    char direction = 0;
    uint32_t last_millis = 0;
    init();

    while(1)
    {   
        for (int i = 0; i < 255; i++)
        {
            if (millis() - last_millis < 5)
            {
                i--; continue;
            }
            direction ? OCR0A++ : OCR0A--;
            last_millis = millis();
        }
        direction = !direction;
    }
}