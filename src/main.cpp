#include "serial.h"
#include "StateMachine.h"
#include "millis.c"
#include "commands.h"
#include "entity.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

// Pin definitions using CrispyPotato Shield:
#define LED_RED   3  // (PORTB BIT 3)
#define LED_GREEN 1  // (PORTB BIT 1, pin 9)
#define LED_BLUE  2  // (PORTB BIT 2, pin 10)
#define BUTTON_1  0  // (PORTB BIT 0, pin 8)
#define BUTTON_2  4  // (PORTB BIT 4, pin 12)

// Preferences
#define LED_BLINK_INTERVAL_MS 1000

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

// Volatile byte used for interrupts
volatile uint8_t ISR_UART_STATE;

void toggle_led_on(ENTITY_LED *led)
{
    PORTB = 1 << led->registry_bit | PORTB;
    led->is_lit = 1;
}

void toggle_led_off(ENTITY_LED *led)
{   
    PORTB = ~(1 << led->registry_bit) & PORTB;
    led->is_lit = 0;
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
    return 0;
}

ISR (USART_RX_vect)
/*
* interrupt service routine, triggered by received chars
* in the USART buffer on UDR0 register.
*/
{
    ISR_UART_STATE = parse_command();
}

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
    // Transition to the state which maps to the char given by the interrupt routine
    sm.transitionTo(ISR_UART_STATE);
}

void init()
{
    // DDRB MUST BE: 00001110 (0 TO LISTEN, 1 TO WRITE)
    // Set DDRB to listen to all pins except bit 1, 2, 3. (0 0 0 0 1 1 1 0)
    DDRB = (7 << 1);

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

    // Temporarily disable interrupt routines and enable millis
    cli();
    init_millis(F_CPU);

    // Enable interrupt routines
    sei();

    // Green light on
    toggle_led_on(&green_led);
}

int main()
{
    // Function pointer variable used to store the pointer given by the StateMachine
    fp_t next_state;
    
    // Initialize the runtime environment
    init();
    
    // Program main loop - poll the StateMachine instance
    // for the next function call ( State ) in line.
    while(1)
    {
        next_state = sm.next();
        next_state();
    }
}