#include <rcc.h>
#include <gpio.h>
#include <led.h>
#include <button.h>
#include <cortex_m3.h>
#include <bitband.h>
#include <flash.h>
#include <stm32f103c8t6.h>

void SysTick_Handler()
{
  GPIOA.ODR.BITS.b5 =!GPIOA.ODR.BITS.b5;
}

struct
{
  unsigned char Count;
  unsigned char State;
  
} IRCapture;

void main()
{
  RCC_BITBAND.APB2_ENR.IOPA = 1;
  GPIOA.ODR.REG = BIT3 | BIT5;
  GPIOA_Enable();
  GPIO_Mode(&GPIOA,BIT1| BIT3 | BIT5, GPIO_MODE_OUTPUT_PUSHPULL_2MHz);
  GPIO_Mode(&GPIOA, BIT5, GPIO_MODE_INPUT_PULL);
  
  RCC_BITBAND.APB2_ENR.IOPC = 1;
  GPIO_Mode(&GPIOC, BIT13, GPIO_MODE_OUTPUT_PUSHPULL_2MHz);
  
  STK_Init(400); //System tick 50uS
  while(1)
  {
  }
}