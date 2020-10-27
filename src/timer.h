#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

uint32_t millis();
void timer_init();

#endif
#ifdef __cplusplus
}
#endif