/* main_b12_B_S03.c - Buoi 12 · Problem B (LED PC13 by TIM->DMA) · Stage 0_3 (raw DSL).
 *
 * One file = one (problem x stage) cell - no #ifdef switching. Select the
 * cell to run by keeping exactly ONE main_b12_*.c in the build (IAR GUI:
 * include/exclude from build). This file expects profile
 * DMA_B12_S02_03_BareMetal (define STM32F103xB only - no HAL compiled).
 *
 * Final rung: nothing of the vendor's remains. Names come from the
 * hand-written maps (dma.h - new for Buoi 12 - plus advanced_timer.h,
 * gpio.h, rcc.h) and the placed instances in stm32f103c8t6.h.
 * Same physical writes as 0_2 - the fingerprint must not move by one bit.
 * PASSED on board: CCR5=0x00000AB1, DIER=0x100, PSC=7999, ARR=399,
 * LED blinks with the core halted.
 */

#include <stdint.h>
#include <stm32f103c8t6.h>   /* hand-written map: RM0008 only, zero vendor */

/* Same pattern data as 0_1/0_2 so the fingerprint diff is apples-to-apples. */
static uint32_t g_led_pattern[2] = { (1uL << 13), 0uL };

/* No __root debug pointers this rung: DMA1 / TIM1 / GPIOC are real placed
 * symbols - C-SPY can watch them directly by name. */

/* Same 2 writes as 0_2's GPIO leg, ours by name (gpio.h <- RM0008 ch. 9). */
static void Stage03_Led_Gpio_Init(void)
{
    RCC.APB2_ENR.BITS.IOPC = 1;         /* clock first, always (STANDARD 7)  */

    GPIOC.CRH.BITS.MODE_13 = 2;         /* 2 = output, 2 MHz max             */
    GPIOC.CRH.BITS.CNF_13  = 0;         /* 0 = general-purpose push-pull     */
}

/* Same ~13 writes as 0_2's TIM+DMA leg (see the 0_2 trace comment) - the
 * shape is identical, only the names changed owners. CH[4] is RM0008
 * channel 5, the one hard-wired to TIM1_UP (table 78). */
static void Stage03_LedByTimDma(void)
{
    /* -- timer: 8 MHz HSI / 8000 = 1 kHz tick, reload every 400 ticks -- */
    RCC.APB2_ENR.BITS.TIM1 = 1;

    TIM1.ARR = 400u - 1u;               /* update event each 400 ms          */
    TIM1.PSC = 8000u - 1u;              /* 1 tick = 1 ms                     */
    TIM1.RCR = 0u;                      /* fire on every reload              */
    TIM1.EGR.BITS.UG = 1;               /* force update: PSC is a preload,   */
                                        /* latch it NOW (0_2 lesson)         */

    /* -- DMA: memory (pattern) -> peripheral (ODR), circular, word cells -- */
    RCC.AHB_ENR.BITS.DMA1 = 1;

    DMA1.CH[4].CCR.REG = 0;             /* known state; config only while EN=0 */
    DMA1.CH[4].CCR.BITS.DIR   = 1;      /* 1 = read memory, write peripheral */
    DMA1.CH[4].CCR.BITS.CIRC  = 1;      /* never "done": CNDTR auto-reloads  */
    DMA1.CH[4].CCR.BITS.MINC  = 1;      /* walk the pattern                  */
    DMA1.CH[4].CCR.BITS.MSIZE = 2;      /* 2 = 32-bit cells                  */
    DMA1.CH[4].CCR.BITS.PSIZE = 2;      /* (PINC stays 0: ODR never moves)   */
    DMA1.CH[4].CNDTR.BITS.NDT = 2u;     /* two ODR images                    */
    DMA1.CH[4].CPAR = (uint32_t)&GPIOC.ODR;       /* dest (DIR=1: CPAR="to") */
    DMA1.CH[4].CMAR = (uint32_t)g_led_pattern;    /* source                  */
    DMA1.CH[4].CCR.BITS.EN = 1;         /* armed -> CCR = 0x00000AB1;        */
                                        /* waits for TIM1_UP requests        */

    /* -- hook them together and press play -- */
    TIM1.DIER.BITS.UDE = 1;             /* DMA request on update - NOT IRQ   */
    TIM1.CR1.BITS.CEN  = 1;             /* GO                                */

    /* NO poll, NO interrupt: from here the LED blinks with the CPU halted.
     * Watch DMA1.CH[4].CNDTR bounce 2->1->2 and GPIOC.ODR flip in Live Watch. */
}

int main(void)
{
    Stage03_Led_Gpio_Init();
    Stage03_LedByTimDma();

    while (1)
    {
        /* idle: timer + DMA run the show; break here and the LED keeps blinking */
    }
}
