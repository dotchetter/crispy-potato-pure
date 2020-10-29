#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TIMER_H_
#define _TIMER_H_

#define CPU 16000000UL
#define PRESCALER 64UL
#define RATE 1000UL

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

uint32_t millis();
void timer0_init();
void timer2_init();
uint8_t simple_ramp();

#endif
#ifdef __cplusplus
}
#endif