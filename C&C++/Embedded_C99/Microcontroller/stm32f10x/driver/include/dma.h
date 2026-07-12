#ifndef _DMA_H_
#define _DMA_H_

#include <define.h>

/* DMA - Direct Memory Access controller (RM0008 chapter 13).
 *
 * DMA1 has 7 channels. Each channel is the same 5-word block (CCR, CNDTR,
 * CPAR, CMAR, one reserved word = 20 bytes), repeated back-to-back after the
 * two shared status registers (ISR / IFCR) - so the map below models a
 * channel once and stacks 7 of them in an array.
 *
 * Channel numbering: RM0008 counts channels 1..7, C arrays count 0..6 -
 * DMA1.CH[0] is "DMA1_Channel1" in the manual, DMA1.CH[4] is channel 5.
 *
 * EXAMPLE (mem2mem one-shot copy of 8 words, Buoi 12 Problem A):
 *            RCC.AHB_ENR.BITS.DMA1 = 1;          // clock first, always
 *            DMA1.CH[0].CCR.REG = 0;             // configure only while EN=0
 *            DMA1.CH[0].CCR.BITS.MEM2MEM = 1;    // no peripheral, RAM->RAM
 *            DMA1.CH[0].CCR.BITS.MSIZE = 2;      // 2 = 32-bit cells
 *            DMA1.CH[0].CCR.BITS.PSIZE = 2;
 *            DMA1.CH[0].CCR.BITS.MINC  = 1;      // walk both arrays
 *            DMA1.CH[0].CCR.BITS.PINC  = 1;
 *            DMA1.CH[0].CNDTR.BITS.NDT = 8;
 *            DMA1.CH[0].CPAR = (unsigned long)src;
 *            DMA1.CH[0].CMAR = (unsigned long)dst;
 *            DMA1.CH[0].CCR.BITS.EN = 1;         // GO - hardware copies alone
 *            while (!DMA1.ISR.BITS.TCIF1) { }    // (optional) done flag
 */

//                                      |       //Address       Default         Description
typedef struct
{
  BUNION(CCR, unsigned long,                     //0x08+20*(x-1)  0               Channel x configuration register
    EN                                  , 1,    //0             0               Channel enable (config only while 0)
    TCIE                                , 1,    //1             0               Transfer complete interrupt enable
    HTIE                                , 1,    //2             0               Half transfer interrupt enable
    TEIE                                , 1,    //3             0               Transfer error interrupt enable
    DIR                                 , 1,    //4             0               Direction: 0 = read peripheral, 1 = read memory
    CIRC                                , 1,    //5             0               Circular mode (CNDTR auto-reload)
    PINC                                , 1,    //6             0               Peripheral increment mode
    MINC                                , 1,    //7             0               Memory increment mode
    PSIZE                               , 2,    //8:9           0               Peripheral size: 0 = 8-bit, 1 = 16-bit, 2 = 32-bit
    MSIZE                               , 2,    //10:11         0               Memory size:     0 = 8-bit, 1 = 16-bit, 2 = 32-bit
    PL                                  , 2,    //12:13         0               Priority level: 0 low .. 3 very high
    MEM2MEM                             , 1,    //14            0               Memory-to-memory mode (no request needed)
    _reserved                           , 17);
  BUNION(CNDTR, unsigned long,                   //+0x04          0               Channel x number-of-data register
    NDT                                 , 16,   //0:15          0               Data count: down-counts per beat, CIRC reloads it
    _reserved                           , 16);
  unsigned long CPAR;                            //+0x08          0               Channel x peripheral address (full word, no fields)
  unsigned long CMAR;                            //+0x0C          0               Channel x memory address     (full word, no fields)
  unsigned long _reserved;                       //+0x10          -               pad to the 20-byte channel stride
} DMA_CHANNEL_TypeDef;

typedef struct
{
  BUNION(ISR, unsigned long,                     //0x00           0               Interrupt status register (read-only)
    const GIF1                          , 1,    //0             0               Channel 1 global flag (GIF = TCIF | HTIF | TEIF)
    const TCIF1                         , 1,    //1             0               Channel 1 transfer complete flag
    const HTIF1                         , 1,    //2             0               Channel 1 half transfer flag
    const TEIF1                         , 1,    //3             0               Channel 1 transfer error flag
    const GIF2                          , 1,    //4             0               Channel 2 global flag
    const TCIF2                         , 1,    //5             0               Channel 2 transfer complete flag
    const HTIF2                         , 1,    //6             0               Channel 2 half transfer flag
    const TEIF2                         , 1,    //7             0               Channel 2 transfer error flag
    const GIF3                          , 1,    //8             0               Channel 3 global flag
    const TCIF3                         , 1,    //9             0               Channel 3 transfer complete flag
    const HTIF3                         , 1,    //10            0               Channel 3 half transfer flag
    const TEIF3                         , 1,    //11            0               Channel 3 transfer error flag
    const GIF4                          , 1,    //12            0               Channel 4 global flag
    const TCIF4                         , 1,    //13            0               Channel 4 transfer complete flag
    const HTIF4                         , 1,    //14            0               Channel 4 half transfer flag
    const TEIF4                         , 1,    //15            0               Channel 4 transfer error flag
    const GIF5                          , 1,    //16            0               Channel 5 global flag
    const TCIF5                         , 1,    //17            0               Channel 5 transfer complete flag
    const HTIF5                         , 1,    //18            0               Channel 5 half transfer flag
    const TEIF5                         , 1,    //19            0               Channel 5 transfer error flag
    const GIF6                          , 1,    //20            0               Channel 6 global flag
    const TCIF6                         , 1,    //21            0               Channel 6 transfer complete flag
    const HTIF6                         , 1,    //22            0               Channel 6 half transfer flag
    const TEIF6                         , 1,    //23            0               Channel 6 transfer error flag
    const GIF7                          , 1,    //24            0               Channel 7 global flag
    const TCIF7                         , 1,    //25            0               Channel 7 transfer complete flag
    const HTIF7                         , 1,    //26            0               Channel 7 half transfer flag
    const TEIF7                         , 1,    //27            0               Channel 7 transfer error flag
    _reserved                           , 4);
  BUNION(IFCR, unsigned long,                    //0x04           0               Interrupt flag clear register (write 1 to clear)
    CGIF1                               , 1,    //0             0               Clear channel 1 global flag (clears TC/HT/TE too)
    CTCIF1                              , 1,    //1             0               Clear channel 1 transfer complete flag
    CHTIF1                              , 1,    //2             0               Clear channel 1 half transfer flag
    CTEIF1                              , 1,    //3             0               Clear channel 1 transfer error flag
    CGIF2                               , 1,    //4             0               Clear channel 2 global flag
    CTCIF2                              , 1,    //5             0               Clear channel 2 transfer complete flag
    CHTIF2                              , 1,    //6             0               Clear channel 2 half transfer flag
    CTEIF2                              , 1,    //7             0               Clear channel 2 transfer error flag
    CGIF3                               , 1,    //8             0               Clear channel 3 global flag
    CTCIF3                              , 1,    //9             0               Clear channel 3 transfer complete flag
    CHTIF3                              , 1,    //10            0               Clear channel 3 half transfer flag
    CTEIF3                              , 1,    //11            0               Clear channel 3 transfer error flag
    CGIF4                               , 1,    //12            0               Clear channel 4 global flag
    CTCIF4                              , 1,    //13            0               Clear channel 4 transfer complete flag
    CHTIF4                              , 1,    //14            0               Clear channel 4 half transfer flag
    CTEIF4                              , 1,    //15            0               Clear channel 4 transfer error flag
    CGIF5                               , 1,    //16            0               Clear channel 5 global flag
    CTCIF5                              , 1,    //17            0               Clear channel 5 transfer complete flag
    CHTIF5                              , 1,    //18            0               Clear channel 5 half transfer flag
    CTEIF5                              , 1,    //19            0               Clear channel 5 transfer error flag
    CGIF6                               , 1,    //20            0               Clear channel 6 global flag
    CTCIF6                              , 1,    //21            0               Clear channel 6 transfer complete flag
    CHTIF6                              , 1,    //22            0               Clear channel 6 half transfer flag
    CTEIF6                              , 1,    //23            0               Clear channel 6 transfer error flag
    CGIF7                               , 1,    //24            0               Clear channel 7 global flag
    CTCIF7                              , 1,    //25            0               Clear channel 7 transfer complete flag
    CHTIF7                              , 1,    //26            0               Clear channel 7 half transfer flag
    CTEIF7                              , 1,    //27            0               Clear channel 7 transfer error flag
    _reserved                           , 4);
  DMA_CHANNEL_TypeDef CH[7];                     //0x08           -               CH[x-1] = RM0008 channel x (20-byte stride)
} DMA_TypeDef;

/* Bit-band twin: every bit above becomes one 32-bit word in the alias region
 * (0x40020000 -> alias 0x42400000). Whole-word registers (CPAR/CMAR) keep no
 * useful bit meaning there, so they are plain 32-word pads. */
typedef struct
{
  RSTRUCT(CCR, unsigned long,                    //                              Channel x configuration register
    EN                                     ,    //0             0               Channel enable
    TCIE                                   ,    //1             0               Transfer complete interrupt enable
    HTIE                                   ,    //2             0               Half transfer interrupt enable
    TEIE                                   ,    //3             0               Transfer error interrupt enable
    DIR                                    ,    //4             0               Direction
    CIRC                                   ,    //5             0               Circular mode
    PINC                                   ,    //6             0               Peripheral increment mode
    MINC                                   ,    //7             0               Memory increment mode
    PSIZE                               [2],    //8:9           0               Peripheral size
    MSIZE                               [2],    //10:11         0               Memory size
    PL                                  [2],    //12:13         0               Priority level
    MEM2MEM                                ,    //14            0               Memory-to-memory mode
    _reserved                          [17]);
  RSTRUCT(CNDTR, unsigned long,                  //                              Channel x number-of-data register
    NDT                                [16],    //0:15          0               Data count
    _reserved                          [16]);
  unsigned long CPAR[32];                        //                              full-word register - no single-bit use
  unsigned long CMAR[32];                        //                              full-word register - no single-bit use
  unsigned long _reserved[32];                   //                              channel stride pad
} DMA_CHANNEL_BITBAND_TypeDef;

typedef struct
{
  RSTRUCT(ISR, unsigned long,                    //                              Interrupt status register (read-only)
    const GIF1, const TCIF1, const HTIF1, const TEIF1,    //0:3
    const GIF2, const TCIF2, const HTIF2, const TEIF2,    //4:7
    const GIF3, const TCIF3, const HTIF3, const TEIF3,    //8:11
    const GIF4, const TCIF4, const HTIF4, const TEIF4,    //12:15
    const GIF5, const TCIF5, const HTIF5, const TEIF5,    //16:19
    const GIF6, const TCIF6, const HTIF6, const TEIF6,    //20:23
    const GIF7, const TCIF7, const HTIF7, const TEIF7,    //24:27
    _reserved                           [4]);
  RSTRUCT(IFCR, unsigned long,                   //                              Interrupt flag clear register (write 1 to clear)
    CGIF1, CTCIF1, CHTIF1, CTEIF1,               //0:3
    CGIF2, CTCIF2, CHTIF2, CTEIF2,               //4:7
    CGIF3, CTCIF3, CHTIF3, CTEIF3,               //8:11
    CGIF4, CTCIF4, CHTIF4, CTEIF4,               //12:15
    CGIF5, CTCIF5, CHTIF5, CTEIF5,               //16:19
    CGIF6, CTCIF6, CHTIF6, CTEIF6,               //20:23
    CGIF7, CTCIF7, CHTIF7, CTEIF7,               //24:27
    _reserved                           [4]);
  DMA_CHANNEL_BITBAND_TypeDef CH[7];             //                              CH[x-1] = RM0008 channel x
} DMA_BITBAND_TypeDef;

#endif
