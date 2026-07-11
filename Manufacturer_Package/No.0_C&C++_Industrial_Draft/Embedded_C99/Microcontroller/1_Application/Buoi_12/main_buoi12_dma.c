/* main_buoi12_dma.c - Buoi 12, DMA learning app (single main.c, STAGE-switched).
 *
 * Comments in English per STANDARD.md.
 *
 * STAGE picks which API layer compiles (see Roadmap_12.md, section 2):
 *     0_1 = HAL   (priority 0, simplest)
 *     0_2 = CMSIS (+ raw where forced)
 *     0_3 = raw DSL only
 * STAGE is a *string label* that names the mode, not a numeric value.
 *
 * Problem A (mem-to-mem soak test) - PASSED on board (section 19):
 *     DMA1_Channel1 copies 8 words RAM->RAM, then the CPU idles.
 *     No polling, no interrupt (Stage 0 rule). After the core halts,
 *     inspect g_dst[] and DMA1_Channel1->CNDTR in C-SPY (dumpRegs).
 *
 * Problem B (LED PC13, the on-theme one):
 *     TIM1 update event -> DMA1_Channel5 (circular) -> GPIOC->ODR.
 *     The timer sets the pace, DMA hauls the pattern, the CPU stays
 *     out of the loop entirely - the LED blinks while main() idles.
 */

/* Pick the mode: define exactly ONE of these labels (comment out the others).
 * The label name IS the mode tag - no numbers, tested with #ifdef (not #if ==). */
#define STAGE_0_1        /* 0_1 = HAL   (this build)   */
/* #define STAGE_0_2 */  /* 0_2 = CMSIS (+ raw)        */
/* #define STAGE_0_3 */  /* 0_3 = raw DSL only         */

/* Pick the exercise inside the stage - same label mechanism as STAGE. */
/* #define PROBLEM_A */  /* A = mem2mem RAM->RAM       (PASSED, section 19) */
#define PROBLEM_B        /* B = LED PC13 by TIM1_UP -> DMA1_CH5 (this build) */

#include "stm32f1xx_hal.h"   /* pulls hal_conf -> hal_dma, hal_rcc */

#ifdef STAGE_0_1
/* ====================== STAGE 0_1 - HAL ====================== */

#ifdef PROBLEM_A
/* ---------------- Problem A - mem2mem (one-shot) ---------------- */

static DMA_HandleTypeDef hdma;

/* Source block to move, and a zeroed destination we watch fill up. */
static uint32_t g_src[8] = { 0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u,
                             0x55555555u, 0x66666666u, 0x77777777u, 0x88888888u };
static uint32_t g_dst[8] = { 0u };

/* Configure DMA1_Channel1 for a one-shot memory-to-memory copy and fire it.
 * DMA is a "conveyor belt": we declare source / dest / count / cell-size up
 * front, press start, and it moves the block on its own. */
static void Stage01_MemToMem(uint32_t *src, uint32_t *dst)
{
    __HAL_RCC_DMA1_CLK_ENABLE();                        /* turn on DMA1 on AHB   */

    hdma.Instance                 = DMA1_Channel1;
    hdma.Init.Direction           = DMA_MEMORY_TO_MEMORY;
    hdma.Init.PeriphInc           = DMA_PINC_ENABLE;    /* walk the source array */
    hdma.Init.MemInc              = DMA_MINC_ENABLE;    /* walk the dest array   */
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;/* 32-bit cells          */
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_NORMAL;         /* one-shot, not circular*/
    hdma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma);                                /* writes CCR bits       */

    /* For MEMORY_TO_MEMORY, HAL loads CPAR = 1st addr, CMAR = 2nd addr.
     * So here CPAR = src, CMAR = dst. Length 8 -> CNDTR = 8. */
    HAL_DMA_Start(&hdma, (uint32_t)src, (uint32_t)dst, 8u);  /* fires now        */

    /* NO poll, NO wait: the transfer runs to completion on its own.
     * Read g_dst[] (should mirror g_src[]) and CNDTR (should reach 0) in C-SPY. */
}

int main(void)
{
    Stage01_MemToMem(g_src, g_dst);

    while (1)
    {
        /* idle: block already moved; inspect memory/registers in the debugger */
    }
}

#elif defined(PROBLEM_B)
/* ------------- Problem B - LED PC13 by TIM -> DMA (circular) ------------- */

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

#else
#  error "No PROBLEM selected - define PROBLEM_A or PROBLEM_B"
#endif /* PROBLEM_A / PROBLEM_B */

#elif defined(STAGE_0_2)
/* ====================== STAGE 0_2 - CMSIS (+ raw) ====================== */
/* TODO: same job via readable CMSIS register names (DMA1_Channel1->CCR ...). */
#error "STAGE 0_2 not written yet - build DMA_B12_S02_03_BareMetal only once implemented"

#elif defined(STAGE_0_3)
/* ====================== STAGE 0_3 - raw DSL ====================== */
/* TODO: same job via RSTRUCT/BUNION register map (dma.h). */
#error "STAGE 0_3 not written yet"

#else
#  error "No STAGE selected - define STAGE_0_1 / STAGE_0_2 / STAGE_0_3"
#endif
