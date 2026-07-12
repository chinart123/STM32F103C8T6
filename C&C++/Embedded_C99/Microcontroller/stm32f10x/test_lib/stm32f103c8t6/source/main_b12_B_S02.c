/* main_b12_B_S02.c - Buoi 12 · Problem B (LED PC13 by TIM->DMA) · Stage 0_2 (CMSIS).
 *
 * One file = one (problem x stage) cell - no #ifdef switching. Select the
 * cell to run by keeping exactly ONE main_b12_*.c in the build (IAR GUI:
 * include/exclude from build). This file expects profile
 * DMA_B12_S02_03_BareMetal (define STM32F103xB only - no HAL compiled).
 *
 * Support stage for 0_1: same job, vendor functions gone, vendor NAMES only
 * (stm32f103xb.h). Every write is annotated with the HAL call it replaces,
 * traced in stm32f1xx_hal_gpio.c / _tim.c / _dma.c.
 * PASSED on board - exact same fingerprint as 0_1: CCR5=0x00000AB1,
 * DIER=0x100, PSC=7999, ARR=399, LED blinks with the core halted.
 */

#include "stm32f103xb.h"     /* CMSIS device header: names only, no code */

/* Same pattern data as 0_1 so the fingerprint diff is apples-to-apples. */
static uint32_t g_led_pattern[2] = { (1uL << 13), 0uL };

/* C-SPY helpers (same trick as 0_1): the peripheral macros are casts, not
 * symbols - __root pointers give Live Watch real objects to expand. */
__root DMA_Channel_TypeDef * const g_dbg_dma5 = DMA1_Channel5;
__root TIM_TypeDef         * const g_dbg_tim1 = TIM1;

/* What 0_1's GPIO leg did (traced in stm32f1xx_hal_gpio.c):
 *   __HAL_RCC_GPIOC_CLK_ENABLE() -> RCC->APB2ENR |= IOPCEN        (1 write)
 *   HAL_GPIO_Init(GPIOC, &io)    -> CRH pin-13 nibble = MODE 10,
 *                                   CNF 00 (2 MHz push-pull)      (1 write)
 * Total: 2 register writes hiding under ~180 lines of vendor code. */
static void Stage02_Led_Gpio_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;       /* was __HAL_RCC_GPIOC_CLK_ENABLE */

    GPIOC->CRH = (GPIOC->CRH
               & ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13))  /* wipe pin-13 nibble   */
               |   GPIO_CRH_MODE13_1;         /* was HAL_GPIO_Init: MODE13=10  */
                                              /* (output 2 MHz = SPEED_FREQ_LOW*/
                                              /* ), CNF13=00 = OUTPUT_PP       */
}

/* What 0_1's TIM+DMA leg did (traced in stm32f1xx_hal_tim.c / _dma.c):
 *   __HAL_RCC_TIM1_CLK_ENABLE()   -> RCC->APB2ENR |= TIM1EN        (1 write)
 *   HAL_TIM_Base_Init(&htim1)     -> CR1 mode bits (all stay 0 here),
 *                                    ARR=399; PSC=7999; RCR=0;
 *                                    EGR=UG to latch PSC now       (4 writes)
 *   __HAL_RCC_DMA1_CLK_ENABLE()   -> RCC->AHBENR |= DMA1EN         (1 write)
 *   HAL_DMA_Init(&hdma)           -> CCR = DIR|CIRC|MINC|word      (1 write)
 *   HAL_DMA_Start(&hdma,src,dst,2)-> CNDTR=2; CPAR; CMAR; CCR|=EN  (4 writes)
 *   __HAL_TIM_ENABLE_DMA(UPDATE)  -> DIER |= UDE                   (1 write)
 *   HAL_TIM_Base_Start(&htim1)    -> CR1 |= CEN - GO               (1 write)
 * Total: ~13 register writes hiding under ~900 lines of vendor code. */
static void Stage02_LedByTimDma(void)
{
    /* -- timer: 8 MHz HSI / 8000 = 1 kHz tick, reload every 400 ticks -- */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;       /* was __HAL_RCC_TIM1_CLK_ENABLE */

    /* was HAL_TIM_Base_Init: CounterMode=UP, ClockDivision=DIV1 and
     * AutoReloadPreload=DISABLE are all CR1 bits = 0 -> nothing to write. */
    TIM1->ARR = 400u - 1u;                    /* was Init.Period: beat = 400 ms */
    TIM1->PSC = 8000u - 1u;                   /* was Init.Prescaler: 1 ms tick  */
    TIM1->RCR = 0u;                           /* was Init.RepetitionCounter     */
    TIM1->EGR = TIM_EGR_UG;                   /* HAL tail: force update event   */
                                              /* so PSC leaves preload NOW      */

    /* -- DMA: memory (pattern) -> peripheral (ODR), circular, word cells -- */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;         /* was __HAL_RCC_DMA1_CLK_ENABLE  */

    DMA1_Channel5->CCR &= ~DMA_CCR_EN;        /* HAL_DMA_Init precondition:     */
                                              /* configure only while disabled  */
    DMA1_Channel5->CCR = DMA_CCR_DIR          /* was Init.Direction = MEM2PERIPH*/
                       | DMA_CCR_CIRC         /* was Init.Mode = CIRCULAR       */
                       | DMA_CCR_MINC         /* was Init.MemInc = ENABLE       */
                       | DMA_CCR_MSIZE_1      /* was Init.MemDataAlignment=WORD */
                       | DMA_CCR_PSIZE_1;     /* was Init.PeriphDataAlignment   */
                                              /* PINC off, Priority=LOW,        */
                                              /* interrupts off = bits left 0   */
    DMA1_Channel5->CNDTR = 2u;                /* was HAL_DMA_Start count arg    */
    DMA1_Channel5->CPAR  = (uint32_t)&GPIOC->ODR;  /* dest   (DIR=1: CPAR=dst)  */
    DMA1_Channel5->CMAR  = (uint32_t)g_led_pattern;/* source                    */
    DMA1_Channel5->CCR  |= DMA_CCR_EN;        /* armed -> CCR = 0x00000AB1;     */
                                              /* waits for TIM1_UP requests     */

    /* -- hook them together and press play -- */
    TIM1->DIER |= TIM_DIER_UDE;               /* was __HAL_TIM_ENABLE_DMA:      */
                                              /* DMA request, NOT interrupt     */
    TIM1->CR1  |= TIM_CR1_CEN;                /* was HAL_TIM_Base_Start - GO    */

    /* NO poll, NO interrupt: from here the LED blinks with the CPU halted.
     * Watch DMA1_Channel5->CNDTR bounce 2->1->2 and ODR flip in Live Watch. */
}

int main(void)
{
    Stage02_Led_Gpio_Init();
    Stage02_LedByTimDma();

    while (1)
    {
        /* idle: timer + DMA run the show; break here and the LED keeps blinking */
    }
}
