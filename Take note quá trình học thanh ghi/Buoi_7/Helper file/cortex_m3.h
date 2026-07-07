#ifndef _CORTEX_M3_H_
#define _CORTEX_M3_H_

typedef struct
{
  union
  {
    unsigned long REG;
    struct
    {
      unsigned long ENABLE              : 1;
      unsigned long TICK_INT            : 1;
      unsigned long CLK_SOURCE          : 1;
      unsigned long _reserved           : 13;
      unsigned long COUNT_FLAG          : 1;
      unsigned long _reserved1          : 15;
    } BITS;
  } CTRL;
  unsigned long LOAD;           //only 24 bit low is used
  unsigned long VAL;            //...
  unsigned long CALIB;          //...
} STK_TypeDef;

#define STK             (*((STK_TypeDef*)0xE000E010))

#endif