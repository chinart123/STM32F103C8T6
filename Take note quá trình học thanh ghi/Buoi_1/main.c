#include <rcc.h>
typedef struct
{
  union //CRL reg
  {
    unsigned long REG;
    struct 
    {
      unsigned long MODE_0 :  2;
      unsigned long  CNF_0 :  2;
      unsigned long MODE_1 :  2;
      unsigned long  CNF_1 :  2;
      unsigned long MODE_2 :  2;
      unsigned long  CNF_2 :  2;
      unsigned long MODE_3 :  2;
      unsigned long  CNF_3 :  2;
      unsigned long MODE_4 :  2;
      unsigned long  CNF_4 :  2;
      unsigned long MODE_5 :  2;
      unsigned long  CNF_5 :  2;
      unsigned long MODE_6 :  2;
      unsigned long  CNF_6 :  2;
      unsigned long MODE_7 :  2;
      unsigned long  CNF_7 :  2;
    } BITS;
  } CRL;
  
  union
  {
    unsigned long REG;
    struct 
    {
      unsigned long MODE_8 :  2;
      unsigned long  CNF_8 :  2;
      unsigned long MODE_9 :  2;
      unsigned long  CNF_9 :  2;
      unsigned long MODE_10 :  2;
      unsigned long  CNF_10 :  2;
      unsigned long MODE_11 :  2;
      unsigned long  CNF_11 :  2;
      unsigned long MODE_12 :  2;
      unsigned long  CNF_12 :  2;
      unsigned long MODE_13 :  2;
      unsigned long  CNF_13 :  2;
      unsigned long MODE_14 :  2;
      unsigned long  CNF_14 :  2;
      unsigned long MODE_15 :  2;
      unsigned long  CNF_15 :  2;
    } BITS;
  }CRH;
  union
  {
    unsigned long REG;
    struct
    {
      unsigned long b0 : 1;
      unsigned long b1 : 1;
      unsigned long b2 : 1;
      unsigned long b3 : 1;
      unsigned long b4 : 1;
      unsigned long b5 : 1;
      unsigned long b6 : 1;
      unsigned long b7 : 1;
      unsigned long b8 : 1;
      unsigned long b9 : 1;
      unsigned long b10 : 1;
      unsigned long b11 : 1;
      unsigned long b12 : 1;
      unsigned long b13 : 1;
      unsigned long b14 : 1;
      unsigned long b15 : 1;
      unsigned long _reserved : 16;
    } BITS;
  }IDR;
  
  union
  {
    unsigned long REG;
    struct 
    {
      unsigned long b0 : 1;
      unsigned long b1 : 1;
      unsigned long b2 : 1;
      unsigned long b3 : 1;
      unsigned long b4 : 1;
      unsigned long b5 : 1;
      unsigned long b6 : 1;
      unsigned long b7 : 1;
      unsigned long b8 : 1;
      unsigned long b9 : 1;
      unsigned long b10 : 1;
      unsigned long b11 : 1;
      unsigned long b12 : 1;
      unsigned long b13 : 1;
      unsigned long b14 : 1;
      unsigned long b15 : 1;
      unsigned long _reserved : 16;
    } BITS;
  } ODR;
  union
  {
    unsigned long REG;
    struct 
    {
      union
      {
        unsigned short REG;
        struct 
        {
          unsigned short b0 : 1;
          unsigned short b1 : 1;
          unsigned short b2 : 1;
          unsigned short b3 : 1;
          unsigned short b4 : 1;
          unsigned short b5 : 1;
          unsigned short b6 : 1;
          unsigned short b7 : 1;
          unsigned short b8 : 1;
          unsigned short b9 : 1;
          unsigned short b10 : 1;
          unsigned short b11 : 1;
          unsigned short b12 : 1;
          unsigned short b13 : 1;
          unsigned short b14 : 1;
          unsigned short b15 : 1;
        } BITS;
      }BSR;
      
      union
      {
        unsigned short REG;
        struct 
        {
          unsigned short b0 : 1;
          unsigned short b1 : 1;
          unsigned short b2 : 1;
          unsigned short b3 : 1;
          unsigned short b4 : 1;
          unsigned short b5 : 1;
          unsigned short b6 : 1;
          unsigned short b7 : 1;
          unsigned short b8 : 1;
          unsigned short b9 : 1;
          unsigned short b10  : 1;
          unsigned short b11  : 1;
          unsigned short b12  : 1;
          unsigned short b13  : 1;
          unsigned short b14  : 1;
          unsigned short b15  : 1;
        } BITS;
      }BR;
    };//Anonymous bit
  }     BSRR;
  union
  {
    unsigned long REG;
    struct
    {
      unsigned long b0 : 1;
      unsigned long b1 : 1;
      unsigned long b2 : 1;
      unsigned long b3 : 1;
      unsigned long b4 : 1;
      unsigned long b5 : 1;
      unsigned long b6 : 1;
      unsigned long b7 : 1;
      unsigned long b8 : 1;
      unsigned long b9 : 1;
      unsigned long b10 : 1;
      unsigned long b11 : 1;
      unsigned long b12 : 1;
      unsigned long b13 : 1;
      unsigned long b14 : 1;
      unsigned long b15 : 1;
      unsigned long _reserved : 16;
    }BITS;
  }BRR;
  union
  {
    unsigned long REG;
    struct
    {
      unsigned long b0 : 1;
      unsigned long b1 : 1;
      unsigned long b2 : 1;
      unsigned long b3 : 1;
      unsigned long b4 : 1;
      unsigned long b5 : 1;
      unsigned long b6 : 1;
      unsigned long b7 : 1;
      unsigned long b8 : 1;
      unsigned long b9 : 1;
      unsigned long b10 : 1;
      unsigned long b11 : 1;
      unsigned long b12 : 1;
      unsigned long b13 : 1;
      unsigned long b14 : 1;
      unsigned long b15 : 1;
      unsigned long LCKK : 1;
      unsigned long _reserved : 15;
    }BITS;
  }LCKR;
} GPIO_TypeDef;

__root __no_init GPIO_TypeDef GPIOB @ 0x40010C00; 
__root __no_init GPIO_TypeDef GPIOA @ 0x40010800;
__root __no_init GPIO_TypeDef GPIOC @ 0x40011000;

// Add this line after the GPIOB declaration in main.c
// normally, there will a  system_stm32f10x.h or stm32f10x.h declare this RCC.
// If we already include stm32f10x.h, there would be no need to write this line.
__root __no_init RCC_TypeDef RCC @ 0x40021000;

void main()
{
  //turn on clock for GPIOB
  RCC.APB2_ENR.BITS.IOPB = 1; // or write RCC.APB2_ENR.REG |= (1 << 3); // IOPB is the third bit trong APB2_ENR 
  //config GPIOB.b12 to OUTPUT
  GPIOB.CRH.BITS.MODE_12 = 3;
  GPIOB.CRH.BITS.CNF_12  = 0;
  while(1)
  {
    
    //Blink led 1
    //GPIOB.ODR.REG = ~GPIOB.ODR.REG;
    //GPIOB.ODR.BITS.b12 = !GPIOB.ODR.BITS.b12;
    //for (unsigned long i =0; i< 500000 ; i++);
    
    //Blink led 2
    
    //RESET
    GPIOB.BSRR.REG = (1UL << 12); 
    for (unsigned long i =0; i< 500000 ; i++);
  
//    SET
//    GPIOB.BRR.REG  = (1UL << 12); 
//    for (unsigned long i =0; i< 500000 ; i++);
//    or write, BRR is just the inverse of BSRR, but BSRR can be both SET and RESET
    GPIOB.BSRR.REG = (1UL << 12 + 16); // Look at data sheet
    for (unsigned long i =0; i< 500000 ; i++);
  }
}
