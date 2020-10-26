#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#define COMPARE_MATCHES_PER_SECOND 100
#define PRESC 1024
#define OUTPUT_COMP (F_CPU / PRESC / COMPARE_MATCHES_PER_SECOND)-1

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
* a formula yields the number to feed in to  the OCR0A/B
* output compare registers.
*
* The fomrula to calculate this value is as follows:
* (F_CPU / PRESCALER / COMPARE_MATCHES_PER_SECOND) - 1
*/
{
	// Set TIMER0 mode to CTC (Clear Timer on Compare) by setting the WGM01 to 1
	TCCR0A |= (1 << WGM01);	// binary: 0b00000010

	// Set the prescaler to 1024 by toggling CS00 and CS02 in TCCR0B, in binary: 0b00000101
	TCCR0B |= (1 << CS02) | (1 << CS00);

	// Set the output compare register to defined value
	OCR0A = 155; //|= OUTPUT_COMP;

	// Enable the compare match interupt for timer0
	TIMSK0 |= (1 << TOIE0);
}	


