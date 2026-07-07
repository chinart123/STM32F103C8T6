#ifndef _CONFIG_H_
#define _CONFIG_H_

//config !(led.h)
#include <gpio.h> 
//#define CFG_LED(Value) GPIOB.ODR.BITS.b12 = !(Value)

#define NUMBER_OF_LED 4

void CFG_LED(unsigned int LedIndex, unsigned int Value);


#endif