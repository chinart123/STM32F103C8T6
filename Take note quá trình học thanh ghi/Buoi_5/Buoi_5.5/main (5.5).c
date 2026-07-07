#include <rcc.h>
#include <gpio.h>
#include <led.h>
#include <button.h>
//__no_init Led_TypeDef Led;

__root unsigned char xx;
void main()
{
  /*
  @brief 
   If choose PB7, 
   remember to change in <config.h> #define READ_BUTTON()                (GPIOB.IDR.BITS.b7)
  @
  */
  //turn on clock for GPIOB
  RCC.APB2_ENR.BITS.IOPB = 1;
  //config GPIOB.b12 to OUTPUT
  GPIO_Mode(&GPIOB, 1UL << 12, GPIO_MODE_OUTPUT_PUSHPULL_10MHz);
//  GPIOB.ODR.BITS.b7 = 1;
//  GPIO_Mode(&GPIOB, 1UL << 7, GPIO_MODE_INPUT_PULL);
  
  /*
  @brief 
   If choose PA0, 
   remember to change in <config.h> #define READ_BUTTON()                (GPIOA.IDR.BITS.b0)
  @
  */
  //turn on clock for GPIOA
  RCC.APB2_ENR.BITS.IOPA = 1;
  //  //config GPIOA.b0 to OUTPUT
  GPIO_Mode(&GPIOA, 1UL << 12, GPIO_MODE_OUTPUT_PUSHPULL_10MHz);
  GPIOA.ODR.BITS.b0 = 1;
  GPIO_Mode(&GPIOA, 1UL << 0, GPIO_MODE_INPUT_PULL);
  
  //init led
  //Led_Begin_0();
  
  //init button
  Button_Begin();
  
  ButtonAccel_TypeDef ButtonAccel =
  {
    .HoldTime = 100,
    .MaxHoldTime = 100,
    .MinHoldTime = 11,
    .SubStep = 10,
  };
  
  while(1)
  {
    Button_Process();
          
    //Led_Process_0((void*)0);
    
    //for( unsigned long i =0; i <4579; i++); // delay at 4ms x 10 = 40ms
    for( unsigned long i =0; i <22894; i++); // delay at 20ms x 10 = 200ms
    
//    if (Button_HoldAccel (&ButtonAccel))
//      GPIOB.ODR.BITS.b12 = !GPIOB.ODR.BITS.b12;
    if (Button_Hold(100, 1))
    {
      xx = 4;
    }
//    if (Button_Check(BUTTON_STATUS_FALL, 1))
//    {
//      xx = 1;
//    }
    if (Button_Check(BUTTON_STATUS_RISE, 0))
    {
      xx = 2;
    }
    if (Button_Press())
    {
      xx = 3;
    } 
  }
}