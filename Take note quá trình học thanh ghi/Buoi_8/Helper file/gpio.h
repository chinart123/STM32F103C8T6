#ifndef _GPIO_H_
#define _GPIO_H_

#include <rcc.h>
#include <define.h>

//                                      |       //Address       Default         Description
typedef struct
{
  BUNION(CRL, unsigned long,
       MODE_0                           ,  2,   //0-1           0               0: Input mode (reset state)
                                                //                              1: Output mode, max speed 10 MHz.
                                                //                              2: Output mode, max speed 2 MHz.
                                                //                              3: Output mode, max speed 50 MHz.         
        CNF_0                           ,  2,   //2-3           1               In input mode:
                                                //                                      0: Analog mode
                                                //                                      1: Floating input (reset state)
                                                //                                      2: Input with pull-up / pull-down
                                                //                              In output mode:
                                                //                                      0: General purpose output push-pull
                                                //                                      1: General purpose output Open-drain
                                                //                                      2: Alternate function output Push-pull
                                                //                                      3: Alternate function output Open-drain
       MODE_1                           ,  2,   //
        CNF_1                           ,  2,   //
       MODE_2                           ,  2,   //
        CNF_2                           ,  2,   //
       MODE_3                           ,  2,   //
        CNF_3                           ,  2,   //
       MODE_4                           ,  2,   //
        CNF_4                           ,  2,   //
       MODE_5                           ,  2,   //
        CNF_5                           ,  2,   //
       MODE_6                           ,  2,   //
        CNF_6                           ,  2,   //
       MODE_7                           ,  2,   //
        CNF_7                           ,  2);  //
BUNION(CRH, unsigned long,
       MODE_8                           ,  2,   //
        CNF_8                           ,  2,   //
       MODE_9                           ,  2,   //
        CNF_9                           ,  2,   //
       MODE_10                          ,  2,   //
        CNF_10                          ,  2,   //
       MODE_11                          ,  2,   //
        CNF_11                          ,  2,   //
       MODE_12                          ,  2,   //
        CNF_12                          ,  2,   //
       MODE_13                          ,  2,   //
        CNF_13                          ,  2,   //
       MODE_14                          ,  2,   //
        CNF_14                          ,  2,   //
       MODE_15                          ,  2,   //
        CNF_15                          ,  2);  //

  BUNION(IDR, unsigned long,
         xBITS_B(b,0,15,1),                     //
         _reserved                      , 16);  //
  
  BUNION(ODR, unsigned long,
         xBITS_B(b,0,15,1),                     //
         _reserved                      , 16);  //
  union
  {
    unsigned long REG;
    struct 
    {
      xBUNION(BSR,unsigned short,b,0,15,1);
      xBUNION(BR,unsigned short,b,0,15,1);
    };//Anonymous bit
  } BSRR;
  BUNION(BRR, unsigned long,
         xBITS_B(b,0,15,1),                     //
         _reserved                      , 16);  //
  BUNION(LCKR, unsigned long,
         xBITS_B(b,0,15,1),                     //
         LOCK                           , 1,    //
         _reserved                      , 15);  //
} GPIO_TypeDef;


typedef struct
{
  RSTRUCT(CRL, unsigned long,
       MODE_0                           [2],    //0-1           0               0: Input mode (reset state)
                                                //                              1: Output mode, max speed 10 MHz.
                                                //                              2: Output mode, max speed 2 MHz.
                                                //                              3: Output mode, max speed 50 MHz.         
        CNF_0                           [2],    //2-3           1               In input mode:
                                                //                                      0: Analog mode
                                                //                                      1: Floating input (reset state)
                                                //                                      2: Input with pull-up / pull-down
                                                //                              In output mode:
                                                //                                      0: General purpose output push-pull
                                                //                                      1: General purpose output Open-drain
                                                //                                      2: Alternate function output Push-pull
                                                //                                      3: Alternate function output Open-drain
       MODE_1                           [2],    //
        CNF_1                           [2],    //
       MODE_2                           [2],    //
        CNF_2                           [2],    //
       MODE_3                           [2],    //
        CNF_3                           [2],    //
       MODE_4                           [2],    //
        CNF_4                           [2],    //
       MODE_5                           [2],    //
        CNF_5                           [2],    //
       MODE_6                           [2],    //
        CNF_6                           [2],    //
       MODE_7                           [2],    //
        CNF_7                           [2]);   //
RSTRUCT(CRH, unsigned long,
       MODE_8                           [2],    //
        CNF_8                           [2],    //
       MODE_9                           [2],    //
        CNF_9                           [2],    //
       MODE_10                          [2],    //
        CNF_10                          [2],    //
       MODE_11                          [2],    //
        CNF_11                          [2],    //
       MODE_12                          [2],    //
        CNF_12                          [2],    //
       MODE_13                          [2],    //
        CNF_13                          [2],    //
       MODE_14                          [2],    //
        CNF_14                          [2],    //
       MODE_15                          [2],    //
        CNF_15                          [2]);   //
                                                
  RSTRUCT(IDR, unsigned long,                   
         xREGS_R(b,0,15,),                      //
         _reserved                      [16]);  //
  
  RSTRUCT(ODR, unsigned long,
         xREGS_R(b,0,15,),                      //
         _reserved                      [16]);  //
  struct
  {
      xRSTRUCT(BSR,unsigned long,b,0,15,);
      xRSTRUCT(BR,unsigned long,b,0,15,);
  } BSRR;
  RSTRUCT(BRR, unsigned long,
         xREGS_R(b,0,15,),                      //
         _reserved                      [16]);  //
  RSTRUCT(LCKR, unsigned long,
         xREGS_R(b,0,15,),                      //
         LOCK                           [1],    //
         _reserved                      [15]);  //
} GPIO_BITBAND_TypeDef;

__root __no_init volatile GPIO_TypeDef GPIOB @ 0x40010C00; 
__root __no_init volatile GPIO_TypeDef GPIOA @ 0x40010800;
//__root __no_init volatile GPIO_TypeDef GPIOC @ 0x40011000;

#include <bitband.h>
__root __no_init volatile GPIO_BITBAND_TypeDef GPIOB_BITBAND @ BITBAND_PERIPHERAL_ADDRESS(0x40010C00,0);
//#define GPIOB(*(volatile GPIO_TypeDef*)0x40010C00) 
//#define GPIOA(*(volatile GPIO_TypeDef*)0x40010800) 



typedef enum{
 
  GPIO_MODE_INPUT_ANALOG                 =  (0<<2) | 0,
  GPIO_MODE_INPUT_FLOAT                  =  (1<<2) | 0,
  GPIO_MODE_INPUT_PULL                   =  (2<<2) | 0,
  
  GPIO_MODE_OUTPUT_PUSHPULL_10MHz        =  (0<<2) | 1,
  GPIO_MODE_OUTPUT_OPEN_10MHz            =  (1<<2) | 1,
  GPIO_MODE_AF_OUTPUT_PUSHPULL_10MHz     =  (2<<2) | 1,
  GPIO_MODE_AF_OUTPUT_OPEN_10MHz         =  (3<<2) | 1,
  
  GPIO_MODE_OUTPUT_PUSHPULL_2MHz         =  (0<<2) | 2,
  GPIO_MODE_OUTPUT_OPEN_2MHz             =  (1<<2) | 2,
  GPIO_MODE_AF_OUTPUT_PUSHPULL_2MHz      =  (2<<2) | 2,
  GPIO_MODE_AF_OUTPUT_OPEN_2MHz          =  (3<<2) | 2,
  
  GPIO_MODE_OUTPUT_PUSHPULL_50MHz        =  (0<<2) | 3,
  GPIO_MODE_OUTPUT_OPEN_50MHz            =  (1<<2) | 3,
  GPIO_MODE_AF_OUTPUT_PUSHPULL_50MHz     =  (2<<2) | 3,
  GPIO_MODE_AF_OUTPUT_OPEN_50MHz         =  (3<<2) | 3,
  
} GPIO_MODE;
// if you want to config PIN 0 and PIN 1:
// that's corresponding to 0x01 | 0x02
void GPIO_Mode(volatile GPIO_TypeDef*, unsigned int PIN, GPIO_MODE Mode);

#define GPIOA_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x04
#define GPIOB_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x08
#define GPIOC_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x10
#define GPIOD_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x20
#define GPIOE_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x40
#define GPIOF_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x80
#define GPIOG_Enable()  *((unsigned long*) (0x40021000 +  0x18)) |= 0x100

#define GPIOA           (*((GPIO_TypeDef*) 0x40010800))
#define GPIOB           (*((GPIO_TypeDef*) 0x40010C00))
#define GPIOC           (*((GPIO_TypeDef*) 0x40011000))
#define GPIOD           (*((GPIO_TypeDef*) 0x40011400))
#define GPIOE           (*((GPIO_TypeDef*) 0x40011800))
#define GPIOF           (*((GPIO_TypeDef*) 0x40011C00))
#define GPIOG           (*((GPIO_TypeDef*) 0x40012000))
#endif     //GPIO_H          
