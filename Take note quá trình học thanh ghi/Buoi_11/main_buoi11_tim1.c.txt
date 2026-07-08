/**
 * @file    main_buoi11_tim1.c
 * @brief   Buoi 11 — TIM1 update (overflow) interrupt blinking the onboard LED.
 *
 * Goal of this lesson: make TIM1 overflow ~2x/second and toggle PC13 from its
 * update ISR. It exercises the FULL chain a peripheral interrupt needs:
 *
 *     RCC clock  ->  timer config (PSC/ARR)  ->  DIER.UIE  ->  CR1.CEN
 *                                                    |
 *                                            NVIC line enable   <-- the CPU gate
 *                                                    |
 *                                          TIM1_UP_IRQHandler runs
 *
 * *** DELIBERATE TEACHING BUG (planted on purpose) ***
 * The NVIC line for TIM1_UP (IRQ #25) is intentionally NOT enabled below.
 * Everything on the timer side is correct, so the timer DOES overflow and DOES
 * raise its request — but the NVIC never forwards it to the core, so the ISR
 * never runs and the LED never blinks. This is the classic "compiles fine,
 * doesn't work" interrupt bug. Capture the registers over SWD to confirm:
 *
 *     RCC.APB2ENR  bit4 (IOPC)=1, bit11 (TIM1)=1   -> clocks on         (correct)
 *     TIM1.DIER    = 0x0001  (UIE=1)               -> timer wants IRQ   (correct)
 *     TIM1.CR1     = 0x0001  (CEN=1)               -> timer running     (correct)
 *     TIM1.SR      = 0x0001  (UIF=1, stuck)        -> overflow happened, never cleared
 *     NVIC ISER0 @ 0xE000E100 bit25 = 0            -> THE CULPRIT: line never enabled
 *
 * Fix = un-comment the single NVIC line marked "BUG FIX" and rebuild.
 *
 * Core clock is 8 MHz HSI (no PLL). TIM1 sits on APB2; with APB2 prescaler = 1,
 * TIM1CLK = PCLK2 = 8 MHz.
 *   overflow period = (PSC+1) * (ARR+1) / TIM1CLK
 *                   = 8000 * 500 / 8e6 = 0.5 s  ->  ~1 Hz visible blink.
 */

#include <stm32f103c8t6.h>

/* PC13 is the Blue Pill onboard LED (active-low). */
#define LED_PIN     (1UL << 13)

/**
 * @brief  Configure PC13 as a 2 MHz push-pull output for the LED.
 * @note   A peripheral is dead until its RCC clock is enabled (STANDARD §7).
 */
static void led_pc13_init(void)
{
    RCC.APB2_ENR.BITS.IOPC = 1;                       /* clock the GPIOC port    */
    GPIO_Mode(&GPIOC, LED_PIN, GPIO_MODE_OUTPUT_PUSHPULL_2MHz);
    GPIOC_BITBAND.ODR.b13  = 1;                       /* active-low -> start OFF */
}

/**
 * @brief  Set TIM1 to overflow every ~0.5 s and request the update interrupt.
 * @note   Uses direct register access: the Level-1 ADVANCED_TIMER_Init() helper
 *         is only declared in the headers, never implemented, so calling it
 *         would fail to LINK. Raw registers keep this lesson self-contained.
 */
static void tim1_overflow_init(void)
{
    RCC.APB2_ENR.BITS.TIM1 = 1;                       /* clock TIM1 (APB2 bit11) */

    TIM1.PSC = 7999u;                                 /* 8 MHz / 8000 = 1 kHz tick */
    TIM1.ARR = 499u;                                  /* 500 ticks   = 0.5 s       */
    TIM1.EGR.BITS.UG  = 1;                            /* load PSC/ARR into shadow  */
    TIM1.SR.BITS.UIF  = 0;                            /* clear the flag UG just set */

    TIM1.DIER.BITS.UIE = 1;                           /* enable UPDATE interrupt   */

    /* ---------------------------------------------------------------------- *
     *  FIX (was the planted bug): enable the NVIC line for TIM1_UP (IRQ #25). *
     *  The chip has no NVIC register map yet, so we write ISER0 (0xE000E100)  *
     *  directly. Without this gate the timer overflows but the core never     *
     *  sees the request, so TIM1_UP_IRQHandler never runs and the LED stays   *
     *  dark. Register capture over SWD showed ISER0 bit25 = 0 -> the culprit. *
     * ---------------------------------------------------------------------- */
    *((volatile unsigned long*)0xE000E100) |= (1UL << 25);   /* NVIC: enable TIM1_UP (IRQ #25) */

    TIM1.CR1.BITS.CEN = 1;                            /* start the counter         */
}

/**
 * @brief  TIM1 update ISR: clear the flag and toggle the LED.
 * @note   Overrides the PUBWEAK symbol in the IAR startup file. If the NVIC
 *         line is disabled (the planted bug), this never runs.
 */
void TIM1_UP_IRQHandler(void)
{
    TIM1.SR.BITS.UIF = 0;                             /* clear or it re-fires forever */
    GPIOC_BITBAND.ODR.b13 = !GPIOC_BITBAND.ODR.b13;   /* atomic single-bit toggle     */
}

void main(void)
{
    led_pc13_init();
    tim1_overflow_init();

    while (1)
    {
        /* All the work happens in TIM1_UP_IRQHandler — nothing to do here. */
    }
}
