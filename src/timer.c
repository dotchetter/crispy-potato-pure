
#include "timer.h"
#define CPU 16000000UL
#define PRESCALER 1024UL
#define RATE 100UL

volatile uint32_t timer0_ms;

ISR (TIMER0_COMPA_vect)
{
    timer0_ms++;
}

void timer_init()
/*
* The oscillator in the ATmega328p is running at 
* 16,000,000 Hz (16MHz). Since the OCR0A/B registers
* are only 8 bit, an overflow trigger on this register
* is not ideal due to the very frequent trigger point
* using this kind of interrupt. 
*
* This implementation is using Timer Compare by comparing
* the TCNT0 register with OCR0A repeatedly where only a 
* match between the two (counter and fixed value) will
* trigger an interrupt. 
* 
* Using the overflow method (normal operation), the TCNT0 
* will overflow every 16*10^6 / 256 which is roughly an 
* overflow every 16 microseconds. To mitigate this short
* timeframe, CTC or Clear Timer on Compare is used.
* 
* PRESCALER
* The prescaler serves as a divisor, and including it in
* a formula yields the number to feed in to the OCR0A/B
* output compare registers. As soon as the counter TCNT0
* reaches this number it will trigger the interrupt, telling
* us that the desired amount of time now has passed and we 
* can perform actions on that que.
*
* The fomrula to calculate output compare is as follows:
* output_compare = (f_cpuclk / (prescale_val * rate) - 1
* where the (-1) accounts for the rollover event when the 
* counter reaches the desired number.
* 
* The 'rate' represents the amount of counts that equals the
* desired period of time. 
* 
*/
{
	// Set TIMER0 mode to 2 (CTC (Clear Timer on Compare)) by setting the WGM01 bit to 1. In binary: 0b00000010
	TCCR0A |= (1 << WGM01);

	// Set the prescaler (clock divisor) to 1024 by toggling CS00 and CS02 in TCCR0B. In binary: 0b00000101
	TCCR0B |= (1 << CS02) | (1 << CS00); 

	// Calculate the output compare value
	OCR0A |= (CPU / PRESCALER / RATE - 1);

	// Enable the compare match interupt for timer0
	//TIMSK0 |= (1 << OCIE0A);
}

uint32_t millis()
/*
* Resembles the millis function in the Arduino universe.uint8_t
* Returns the amount of milliseconds since device boot.
* The counter overflows at the maximum theoretical capacity
* of an unsigned 32 bit integer of 4,294,967,295 which 
* equates to 49,71 days or 1,193.05 hours or 
* 4,294,967,3 seconds.
*
* The capture of the current value of the volatile variable
* timer0_ms is cloned as to not return a volatile value.
*/
{
	uint32_t timer0_current_ms;

	// Temporarily disable interrupts during this process
	ATOMIC_BLOCK(ATOMIC_FORCEON) 
	{
		timer0_current_ms = timer0_ms;
	}
	return timer0_current_ms;
}