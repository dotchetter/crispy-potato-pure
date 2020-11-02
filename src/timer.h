#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

volatile uint32_t timer2_ms;
volatile uint8_t timer2_ctc_match;

const uint8_t simple_ramp();
void timer0_init();
void timer2_init(uint32_t cpu_clk);
unsigned long millis();

#endif
#ifdef __cplusplus
}
#endif