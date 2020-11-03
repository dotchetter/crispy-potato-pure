#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "helpers.h"
#include "timer.h"
#include "StateMachine.h"
#include "entity.h"
#include "led.h"


//Pin definitions using CrispyPotato Shield:
#define LED_PWM_REG_BIT 6   // (PORTD BIT 6, pin 6)
#define KEY_1_REG_BIT   2   // (PORTD BIT 2, pin 2)


// Compiler contract - methods defined below
void idle_state();
void led_pulse_state();
void led_potentiometer_state();
void led_flashing_state();
void off_state();


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
volatile uint32_t PWM_INTERRUPT_DUTY_CYCLE;


void init()
/*
* Initialize the runtime-environment on the MCU. Steps are as defined:
*
* # 0: Set DDRB to listen to all pins except bit 1, 2, 3. (0 0 0 0 1 1 1 0)
* # 1: Enable pin 6 as output for PWM LED
* # 2: Enable pin 2 as input for pushbutton
* # 3: Enable interrupt over USART receive (USART_RX, ATmega328p datasheet 12.1)
* # 4: Set ADMUX[0:3] to 0xFF to set up ADC0 as input with VCC as reference voltage
* # 5: Enable ADC | set ADC prescaler to 8 | ADC Interrupt Enable | Left-adjust result
* # 6: Disable Digital buffer on pin A0
* # 7: Initialize UART. Used to receive commands
* # 8: Add states to the statemachine + create state-chain + Set the initial Static State to PULSE_LED
* # 9: Configure entities - sets values and port pointers to entities
* # 10: Temporarily disable interrupt routines and enable timer0 for PWM and timer2 for millis
*/
{   
    DDRB |= (_BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3));                  // # 0
    DDRD |= _BV(PORTD6);                                                // # 1
    DDRD &= ~(_BV(PORTD2));                                             // # 2
    UCSR0B |= (1 << RXCIE0);                                            // # 3
    ADMUX |= _BV(REFS0);                                                // # 4

    ADCSRA |= _BV(ADIE) | _BV(ADEN) | _BV(ADPS0)                        // # 5
           | _BV(ADPS1) | _BV(ADLAR);
    
    DIDR0 |= _BV(ADC0D);                                                // # 6

    uart_init();                                                        // # 7

    stateMachine.addState(PULSE_LED, &led_pulse_state);                 // # 8
    stateMachine.addState(POTENTIOMETER_LED, &led_potentiometer_state);
    stateMachine.addState(FLASH_LED, &led_flashing_state);
    stateMachine.addState(OFF, &off_state);

    stateMachine.setChainedState(PULSE_LED, POTENTIOMETER_LED);
    stateMachine.setChainedState(POTENTIOMETER_LED, FLASH_LED);
    stateMachine.setChainedState(FLASH_LED, OFF);
    stateMachine.setChainedState(OFF, PULSE_LED);

    stateMachine.setStaticState(PULSE_LED);

    pwm_led.registry_bit = PORTD6;                                      // # 9
    pwm_led.is_active = 0;
    pwm_led.port = &OCR0A;
    pwm_led.data_direction_register = &DDRD;

    key_1.registry_bit = PORTD2;
    key_1.port = &PIND;
    key_1.last_updated_ms = 0;
    key_1.debounce_ms = 50;
    key_1.long_press_trigger_ms = 200;

    cli();                                                              // # 10
    timer2_init(16000000UL);
    timer0_init();
    sei();
}


// Interrupt Service Routines
 
ISR (USART_RX_vect)
/*
* interrupt service routine, triggered by received chars
* in the USART buffer on UDR0 register.
*/
{
    UART_INTERRUPT_TRIGGERED = 1;
}


ISR (ADC_vect)
{
    PWM_INTERRUPT_DUTY_CYCLE = ADC;
}


// --- State functions --- //
void idle_state()
/*
* The idle state is the main state of this firmware.
* The StateMachine instance will automatically resort
* to this state when a transition to another state is
* complete. This is to provide for a continuous ability
* to read button keys or any other resource. 
*
* The Static State is stored in the StateMachine as 
* a sticky-note to keep track of the state the machine
* just entered. The idea is to only mutate this state
* if the button is pushed and released, and then change
* state to the next state in the chain of states when
* the key was actually released. 
* When the key remains untouched the call stack will
* automatically return to this function per the design
* described above- however, the previous state should
* again be transitioned to as to keep the device doing
* whatever it was told to do the last time the button
* press-release-event occured.
*/
{
    static uint8_t uart_desired_state;
    static uint8_t key_clicked_and_released = 0;
    state next_state = stateMachine.getStaticState();

    if (debounceKey(&key_1) && keyClicked(&key_1))
    {
        key_1.is_active = 1;
    }
    
    else
    {
        if (key_1.is_active)
        {
            key_clicked_and_released = 1;
            key_1.is_active = 0;
        }
    }

    if (key_clicked_and_released)
    {
        next_state = stateMachine.getChainedStateForState(stateMachine.getStaticState());
        stateMachine.setStaticState(next_state);
        key_clicked_and_released = 0;
    }
    stateMachine.transitionTo(next_state);
}


void led_pulse_state()
/*
* Pulse the LED in fading fashion. The
* LED fade speed is controlled with the
* potentiometer knob on the device.
*/
{
    initAnalogDigitalConversion();
    uint8_t potentiometer = convert_range(PWM_INTERRUPT_DUTY_CYCLE, 0, 1023, 0, 10);
    
    if (millis() - pwm_led.last_updated_ms > potentiometer)
    {
        analogWrite(&pwm_led, simple_ramp());
        pwm_led.last_updated_ms = millis();
    }
}


void led_potentiometer_state()
{
    initAnalogDigitalConversion();
    analogWrite(&pwm_led, convert_range(PWM_INTERRUPT_DUTY_CYCLE, 0, 1023, 0, 255));
}


void led_flashing_state()
/*
* Flash the LED with full brightness. The
* LED flash speed is controlled with the
* potentiometer knob on the device.
*/
{
    initAnalogDigitalConversion();

    if (millis() - pwm_led.last_updated_ms > PWM_INTERRUPT_DUTY_CYCLE)
    {
        digitalWrite(&pwm_led, pwm_led.is_active);
        pwm_led.is_active = !pwm_led.is_active;
        pwm_led.last_updated_ms = millis();
    }
}


void off_state()
{
    digitalWrite(&pwm_led, LOW);
}


int main()
{
    uint32_t last_millis;
    init();            

    while(1)
    {
        led_pulse_state();
    }
}