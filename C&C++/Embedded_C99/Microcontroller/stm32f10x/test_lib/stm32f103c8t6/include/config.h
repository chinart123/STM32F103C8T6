#ifndef _CONFIG_H_
#define _CONFIG_H_


#define NUMBER_OF_LED 4

void CFG_LED(unsigned int LedIndex, unsigned int Value);



#include <gpio.h>
#define NUMBER_OF_LED_EX           4
#define NUMBER_OF_LED_LED          4
#define TURN_LED(Value)            GPIOB.ODR.BITS.b12 = !(Value)
#define TURN_LED_EX(Value)         GPIOB.ODR.BITS.b12 = !(Value)


//config for button
//#define READ_BUTTON()                (GPIOB.IDR.BITS.b7)
#define READ_BUTTON()                (GPIOA.IDR.BITS.b0)
#endif