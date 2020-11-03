#include "timer.h"


ISR (TIMER2_COMPA_vect)
/*
* Update millis() value
*/
{
	timer2_ms++;
	timer2_ctc_match = 1;
}


void timer2_init(uint32_t cpu_clk)
{
	// Toggle CTC mode for Timer2
	TCCR2A |= _BV(WGM21);
	
	// Set prescaler to 1024
	TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20);

	// Enable interrupt trigger for CTC
	TIMSK2 |= _BV(TOIE1);

	// Configure Output compare register for Timer2 to trigger every 1 ms
  	OCR2A = (cpu_clk / 1024 / 1000 - 1);

	// Enable CTC interrupt
	TIMSK2 |= _BV(OCIE2A);
}


void timer0_init()
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
	// Set TIMER0 mode to Waveform Generation Mode 3 (FAST PWM TOP=0xFF Non-Inverted)
	TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);

	// Set the prescaler (clock divisor) to 64 by toggling CS00 and CS01 in TCCR0B.
	// This yields a frequency of 16*10^6 / (64 * 256) = 977 (rounded up) hz for the PWM LED.
	TCCR0B |= (1 << CS01) | (1 << CS00); 
}


const uint8_t simple_ramp()
/*
* Returns value from 0-255, incrementing
* with each call and automatically overflows
* to 0.
*/
{
	static uint8_t count;
	static uint8_t direction;
	
	if (count == 0)
		direction = 1;
	else if (count == 255)
	    	direction = 0;
	
	direction ? count++ : count--;
	
	return count;
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
* timer2_ms is cloned as to not return a volatile value.
*/
{
	uint32_t timer2_current_ms;

	// Temporarily disable interrupts during this process
	ATOMIC_BLOCK(ATOMIC_FORCEON) 
	{
		timer2_current_ms = timer2_ms;
	}
	return timer2_current_ms;
}