#include "fsm_time.h"

/* The one and only clock of the system: milliseconds since Time_Init(). */
static volatile unsigned int g_ms = 0;

void Time_Init(unsigned int coreHz)
{
    g_ms         = 0;
    STK.LOAD     = (coreHz / 1000u) - 1u;   /* count 1 ms worth of core cycles */
    STK.VAL      = 0u;                       /* clear current value            */
    STK.CTRL.REG = 7u;                       /* ENABLE | TICKINT | core clock  */
}

unsigned int Time_Millis(void)
{
    return g_ms;
}

void Time_Delay(unsigned int ms)
{
    unsigned int start = g_ms;
    while ((g_ms - start) < ms)
    {
        /* wait; the SysTick interrupt keeps advancing g_ms */
    }
}

/* SysTick interrupt handler — runs once every millisecond. */
void SysTick_Handler(void)
{
    g_ms++;
}
