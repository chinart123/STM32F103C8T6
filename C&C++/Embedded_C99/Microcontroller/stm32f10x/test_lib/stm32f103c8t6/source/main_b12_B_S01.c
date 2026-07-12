/* main_b12_B_S01.c - Buoi 12 · Problem B (LED PC13 by TIM->DMA) · Stage 0_1 (HAL).
 *
 * One file = one (problem x stage) cell - no #ifdef switching. Select the
 * cell to run by keeping exactly ONE main_b12_*.c in the build (IAR GUI:
 * include/exclude from build). This file expects profile DMA_B12_S01_HAL
 * (defines STM32F103xB + USE_HAL_DRIVER; compiles hal_dma/gpio/tim.c).
 *
 * Job: TIM1 update event -> DMA1_Channel5 (circular) -> GPIOC->ODR.
 * The timer sets the pace, DMA hauls the pattern, the CPU stays out of the
 * loop entirely - the LED blinks while main() idles (even halted).
 * PASSED on board (section 20). Fingerprint: CCR5=0x00000AB1, CNDTR 2<->1,
 * CPAR=0x4001100C, CMAR=&g_led_pattern, DIER=0x100, PSC=7999, ARR=399.
 */

#include "stm32f1xx_hal.h"   /* pulls hal_conf -> hal_dma, hal_gpio, hal_tim */

static DMA_HandleTypeDef hdma;
static TIM_HandleTypeDef htim1;

/* Two whole-word ODR images, replayed forever by circular DMA:
 *   [0] = bit13 set   -> PC13 high -> LED OFF (Blue Pill LED is active-low)
 *   [1] = all clear   -> PC13 low  -> LED ON
 * Word-writes to ODR also clear the other PORTC bits - fine, PC13 is the
 * only thing wired on this port. */
static uint32_t g_led_pattern[2] = { (1uL << 13), 0uL };

/* Debug-only handles: DMA1_Channel5 / TIM1 are preprocessor macros, invisible
 * to C-SPY - a real pointer variable makes the whole struct watchable (type
 * "*g_dbg_dma5" / "*g_dbg_tim1" in Live Watch). __root (IAR) stops the linker
 * from dead-stripping them: nothing in the code references them. */
__root DMA_Channel_TypeDef * const g_dbg_dma5 = DMA1_Channel5;
__root TIM_TypeDef         * const g_dbg_tim1 = TIM1;

/* PC13 as a plain push-pull output (HAL rung: no raw CRH pokes here). */
static void Stage01_Led_Gpio_Init(void)
{
    GPIO_InitTypeDef io;

    __HAL_RCC_GPIOC_CLK_ENABLE();               /* clock first, always       */

    io.Pin   = GPIO_PIN_13;
    io.Mode  = GPIO_MODE_OUTPUT_PP;
    io.Pull  = GPIO_NOPULL;
    io.Speed = GPIO_SPEED_FREQ_LOW;             /* 2 MHz is plenty for a LED */
    HAL_GPIO_Init(GPIOC, &io);
}

/* Conveyor belt with a metronome:
 *   TIM1 overflows every 400 ms and raises a DMA request (DIER.UDE) -
 *   NOT an interrupt. DMA1_Channel5 (the channel hard-wired to TIM1_UP,
 *   RM0008 table 78) then moves ONE pattern word into GPIOC->ODR.
 *   CIRC mode reloads CNDTR when it hits 0, so the belt never stops.
 * Unlike Problem A (rip-through at bus speed), each DMA step now waits
 * for a timer beat - CNDTR is finally SLOW enough to watch count down
 * in C-SPY Live Watch. */
static void Stage01_LedByTimDma(void)
{
    /* -- timer: 8 MHz HSI / 8000 = 1 kHz tick, reload every 400 ticks -- */
    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = 8000u - 1u;  /* 1 tick = 1 ms            */
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.Period            = 400u - 1u;   /* update event each 400 ms */
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0u;          /* TIM1-only field: fire every reload */
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim1);                  /* writes PSC/ARR/CR1        */

    /* -- DMA: memory (pattern) -> peripheral (ODR), circular, word cells -- */
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma.Instance                 = DMA1_Channel5;   /* TIM1_UP lives here   */
    hdma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc           = DMA_PINC_DISABLE;   /* ODR never moves   */
    hdma.Init.MemInc              = DMA_MINC_ENABLE;    /* walk the pattern  */
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_CIRCULAR;    /* never "done": no TC   */
    hdma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma);

    HAL_DMA_Start(&hdma, (uint32_t)g_led_pattern, (uint32_t)&GPIOC->ODR, 2u);

    /* -- hook them together and press play -- */
    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);   /* DIER.UDE = 1          */
    HAL_TIM_Base_Start(&htim1);                     /* CR1.CEN  = 1 - GO     */

    /* NO poll, NO interrupt: from here the LED blinks with the CPU halted.
     * Watch DMA1_Channel5->CNDTR bounce 2->1->2 and ODR flip in Live Watch. */
}

int main(void)
{
    Stage01_Led_Gpio_Init();
    Stage01_LedByTimDma();

    while (1)
    {
        /* idle: timer + DMA run the show; break here and the LED keeps blinking */
    }
}
