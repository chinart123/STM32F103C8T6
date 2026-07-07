#include <rcc.h>
#include <gpio.h>
#include <led.h>

__root __no_init RCC_TypeDef RCC @ 0x40021000;
//void main()
//{
//  //turn on clock for GPIOB
//  RCC.APB2_ENR.BITS.IOPB = 1;
//  //config GPIOB.b12 to OUTPUT
//  GPIO_Mode ( &GPIOB, 1UL << 12, GPIO_MODE_OUTPUT_PUSHPULL_10MHz);
//  
//  
//  while(1)
//  {
//    
//    
//    //Blink led 1
//    //GPIOB.ODR.REG = ~GPIOB.ODR.REG;
//    //GPIOB.ODR.BITS.b12 = !GPIOB.ODR.BITS.b12;
//    //for (unsigned long i =0; i< 500000 ; i++);
//    
//    //Blink led 2
//    
//    //RESET  
//    GPIOB.BSRR.REG = (1UL << 12); 
//    for (unsigned long i =0; i< 500000 ; i++);
//  
////    SET
////    GPIOB.BRR.REG  = (1UL << 12); 
////    for (unsigned long i =0; i< 500000 ; i++);
////    or write, BRR is just the inverse of BSRR, but BSRR can be both SET and RESET
//    GPIOB.BSRR.REG = (1UL << 12 + 16); // Look at data sheet
//    for (unsigned long i =0; i< 500000 ; i++);
//  }
//}




void CFG_LED(unsigned int LedIndex, unsigned int Value)
{
   switch(LedIndex)
   {
     case 0: GPIOB.ODR.BITS.b12 = !Value; break;
     case 1: GPIOB.ODR.BITS.b12 = !Value; break;
     case 2: GPIOB.ODR.BITS.b12 = !Value; break;
   }
}

__root unsigned char blink = 0;
void main()
{
  //turn on clock for GPIOB
  RCC.APB2_ENR.BITS.IOPB = 1;
  //config GPIOB.b12 to OUTPUT
  GPIO_Mode(&GPIOB, 1UL << 12, GPIO_MODE_OUTPUT_PUSHPULL_10MHz);
  
  //init led
  Led_Begin();
  
  //because we are at 20ms , then we only need to pluggin 10 to achieve 200ms blink per cycle
  //  Led_Blink(5,10);
  //now we use a variable to control the led: __root unsigned char blink = 0;
  while(1)
  {
    
//    //Basic blink Led, not used    
//    Led_Process(void*)0);
//    for(unsigned long i = 0; i<500000; i++);
//    GPIOB.ODR.REG = ~GPIOB.ODR.REG;


//    //Blink led 5 times( each times 20ms), then turn off

    if(blink)
    {
      blink = 0;
      Led_Blink(0,5,10);
    }
    
    Led_Process((void*)0);
    for( unsigned long i = 0; i<22894; i++);
    
    
  
  }
}