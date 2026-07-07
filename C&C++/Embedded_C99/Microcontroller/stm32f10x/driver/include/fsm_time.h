#ifndef _FSM_TIME_H_
#define _FSM_TIME_H_

#include <stm32f103c8t6.h>

/*
 * Millisecond time base built on SysTick (interrupt driven).
 *
 *   Time_Init(8000000u);          // 8 MHz core -> 1 ms tick
 *   unsigned int now = Time_Millis();
 *   Time_Delay(500u);             // blocking, only for setup code
 *
 * SysTick fires every 1 ms and increments a free-running counter, so every
 * other module (button, led) can ask "what time is it?" instead of counting
 * ticks itself. This is the ONLY place that owns time.
 *
 * NOTE: this defines SysTick_Handler(). The IAR startup file already lists it
 * as a weak symbol, so ours overrides it automatically. Do not define a second
 * SysTick_Handler anywhere else.
 */

/* Configure SysTick for a 1 ms tick from the given core clock (Hz) and start
   counting. Enables the SysTick interrupt (no NVIC setup needed). */
void         Time_Init(unsigned int coreHz);

/* Milliseconds elapsed since Time_Init(). Wraps after ~49 days. */
unsigned int Time_Millis(void);

/* Busy-wait for 'ms' milliseconds. Use only in setup, never in the main loop. */
void         Time_Delay(unsigned int ms);

#endif /* _FSM_TIME_H_ */
