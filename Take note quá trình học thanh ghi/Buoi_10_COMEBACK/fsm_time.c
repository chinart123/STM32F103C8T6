#include "fsm_time.h"

/* Chiếc đồng hồ duy nhất của hệ thống: số mili-giây kể từ Time_Init(). */
static volatile unsigned int g_ms = 0;

void Time_Init(unsigned int coreHz)
{
    g_ms         = 0;
    STK.LOAD     = (coreHz / 1000u) - 1u;   /* đếm số chu kỳ lõi trong 1 ms   */
    STK.VAL      = 0u;                       /* xoá giá trị hiện tại           */
    STK.CTRL.REG = 7u;                       /* ENABLE | TICKINT | clock lõi   */
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
        /* chờ; ngắt SysTick vẫn tiếp tục tăng g_ms */
    }
}

/* Trình phục vụ ngắt SysTick — chạy mỗi mili-giây một lần. */
void SysTick_Handler(void)
{
    g_ms++;
}
