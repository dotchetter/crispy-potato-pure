#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

volatile uint32_t timer2_ms;

unsigned long millis();
void timer0_init();
void timer2_init(uint32_t cpu_clk);

#endif
#ifdef __cplusplus
}
#endif