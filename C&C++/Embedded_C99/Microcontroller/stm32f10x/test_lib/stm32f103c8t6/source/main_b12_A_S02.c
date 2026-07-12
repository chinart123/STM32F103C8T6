/* main_b12_A_S02.c - Buoi 12 · Problem A (mem2mem) · Stage 0_2 (CMSIS).
 *
 * One file = one (problem x stage) cell - no #ifdef switching. Select the
 * cell to run by keeping exactly ONE main_b12_*.c in the build (IAR GUI:
 * include/exclude from build). This file expects profile
 * DMA_B12_S02_03_BareMetal (define STM32F103xB only - no HAL compiled).
 *
 * Support stage for 0_1: same job, but the vendor's *functions* are gone.
 * Only the vendor's *names* remain - stm32f103xb.h is pure naming (#defines,
 * structs, bit masks), zero behavior. Every write below is annotated with the
 * HAL call it replaces, traced by reading stm32f1xx_hal_dma.c line by line.
 * PASSED on board - exact same fingerprint as 0_1: CCR1=0x00004AC1.
 */

#include "stm32f103xb.h"     /* CMSIS device header: names only, no code */

/* Same data as 0_1 so the fingerprint diff is apples-to-apples. */
static uint32_t g_src[8] = { 0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u,
                             0x55555555u, 0x66666666u, 0x77777777u, 0x88888888u };
static uint32_t g_dst[8] = { 0u };

/* C-SPY helper: DMA1_Channel1 is a cast macro, not a symbol - a __root
 * pointer gives Live Watch a real object to expand ("*g_dbg_dma1"). */
__root DMA_Channel_TypeDef * const g_dbg_dma1 = DMA1_Channel1;

/* What 0_1's two HAL calls actually did (traced in stm32f1xx_hal_dma.c):
 *   __HAL_RCC_DMA1_CLK_ENABLE()  ->  RCC->AHBENR |= DMA1EN          (1 write)
 *   HAL_DMA_Init(&hdma)          ->  CCR = Direction|Inc|Align|Mode (1 write,
 *                                    channel must be disabled first)
 *   HAL_DMA_Start(&hdma,s,d,8)   ->  CNDTR=8; CPAR=src; CMAR=dst;
 *                                    CCR |= EN                      (4 writes)
 * Total: ~7 register writes hiding under ~600 lines of vendor code. */
static void Stage02_MemToMem(uint32_t *src, uint32_t *dst)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;         /* was __HAL_RCC_DMA1_CLK_ENABLE */

    DMA1_Channel1->CCR &= ~DMA_CCR_EN;        /* HAL_DMA_Init precondition:    */
                                              /* configure only while disabled */
    DMA1_Channel1->CCR = DMA_CCR_MEM2MEM      /* was Init.Direction = MEM2MEM  */
                       | DMA_CCR_MSIZE_1      /* was Init.MemDataAlignment=WORD*/
                       | DMA_CCR_PSIZE_1      /* was Init.PeriphDataAlignment  */
                       | DMA_CCR_MINC         /* was Init.MemInc = ENABLE      */
                       | DMA_CCR_PINC;        /* was Init.PeriphInc = ENABLE   */
                                              /* Mode=NORMAL, Priority=LOW,    */
                                              /* interrupts off = bits left 0  */
    DMA1_Channel1->CNDTR = 8u;                /* was HAL_DMA_Start count arg   */
    DMA1_Channel1->CPAR  = (uint32_t)src;     /* source      (DIR=0: CPAR=src) */
    DMA1_Channel1->CMAR  = (uint32_t)dst;     /* destination                   */
    DMA1_Channel1->CCR  |= DMA_CCR_EN;        /* fires now -> CCR = 0x00004AC1 */
    /* NO polling, NO interrupts: read g_dst + CNDTR + ISR.TCIF1 in C-SPY.     */
}

int main(void)
{
    Stage02_MemToMem(g_src, g_dst);

    while (1)
    {
        /* idle on purpose - evidence lives in the registers */
    }
}
