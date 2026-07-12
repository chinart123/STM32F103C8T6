/* main_b12_A_S03.c - Buoi 12 · Problem A (mem2mem) · Stage 0_3 (raw DSL).
 *
 * One file = one (problem x stage) cell - no #ifdef switching. Select the
 * cell to run by keeping exactly ONE main_b12_*.c in the build (IAR GUI:
 * include/exclude from build). This file expects profile
 * DMA_B12_S02_03_BareMetal (define STM32F103xB only - no HAL compiled).
 *
 * Final rung: nothing of the vendor's remains. The functions went away in
 * 0_2; now the *names* go too. The register map is hand-written from RM0008
 * chapter 13 into dma.h with the project's BUNION/RSTRUCT DSL, and the
 * placed instances (DMA1 @ 0x40020000, RCC @ 0x40021000) live in
 * stm32f103c8t6.h. Same physical writes as 0_2 - the fingerprint must
 * not move by a single bit. PASSED on board: CCR1=0x00004AC1.
 */

#include <stdint.h>
#include <stm32f103c8t6.h>   /* hand-written map: RM0008 only, zero vendor */

/* Same data as 0_1/0_2 so the fingerprint diff is apples-to-apples. */
static uint32_t g_src[8] = { 0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u,
                             0x55555555u, 0x66666666u, 0x77777777u, 0x88888888u };
static uint32_t g_dst[8] = { 0u };

/* No __root debug pointer this rung: unlike the vendor's cast-macros,
 * DMA1 is a real placed symbol - C-SPY can watch "DMA1" directly. */

/* Same ~7 writes as 0_2, but every name is ours (dma.h <- RM0008).
 * BITS.field stores read-modify-write the whole CCR - safe here because
 * the channel is disabled and no ISR shares it (STANDARD.md section 2). */
static void Stage03_MemToMem(uint32_t *src, uint32_t *dst)
{
    RCC.AHB_ENR.BITS.DMA1 = 1;          /* clock first, always (STANDARD 7)  */

    DMA1.CH[0].CCR.REG = 0;             /* known state; config only while EN=0 */
    DMA1.CH[0].CCR.BITS.MEM2MEM = 1;    /* RAM->RAM: no peripheral request   */
    DMA1.CH[0].CCR.BITS.MSIZE   = 2;    /* 2 = 32-bit memory cells           */
    DMA1.CH[0].CCR.BITS.PSIZE   = 2;    /* 2 = 32-bit "peripheral" cells     */
    DMA1.CH[0].CCR.BITS.MINC    = 1;    /* walk the dest array               */
    DMA1.CH[0].CCR.BITS.PINC    = 1;    /* walk the source array             */
    DMA1.CH[0].CNDTR.BITS.NDT   = 8u;   /* 8 beats                           */
    DMA1.CH[0].CPAR = (uint32_t)src;    /* source (DIR=0: CPAR is "from")    */
    DMA1.CH[0].CMAR = (uint32_t)dst;    /* destination                       */
    DMA1.CH[0].CCR.BITS.EN = 1;         /* fires now -> CCR = 0x00004AC1     */
    /* NO polling, NO interrupts: read g_dst + CNDTR + ISR.TCIF1 in C-SPY.   */
}

int main(void)
{
    Stage03_MemToMem(g_src, g_dst);

    while (1)
    {
        /* idle on purpose - evidence lives in the registers */
    }
}
