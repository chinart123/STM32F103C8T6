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
 * First increment = "De bai A" (mem-to-mem soak test):
 *     DMA1_Channel1 copies 8 words RAM->RAM, then the CPU idles.
 *     No polling, no interrupt (Stage 0 rule). After the core halts,
 *     inspect g_dst[] and DMA1_Channel1->CNDTR in C-SPY (dumpRegs).
 */

/* Pick the mode: define exactly ONE of these labels (comment out the others).
 * The label name IS the mode tag - no numbers, tested with #ifdef (not #if ==). */
#define STAGE_0_1        /* 0_1 = HAL   (this build)   */
/* #define STAGE_0_2 */  /* 0_2 = CMSIS (+ raw)        */
/* #define STAGE_0_3 */  /* 0_3 = raw DSL only         */

#include "stm32f1xx_hal.h"   /* pulls hal_conf -> hal_dma, hal_rcc */

#ifdef STAGE_0_1
/* ====================== STAGE 0_1 - HAL ====================== */

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
