#include <led.h>
#include <config.h>

__no_init Led_TypeDef Led ;




void Led_Begin()
{
  Led.IsOn = 0;
  CFG_LED(0);
  Led.Times = 0;
  Led.EdgeTime = 0;
  Led.CountEdgeTime = 0;
  
}

//FinishFunction is used to output an event after what doing next
void Led_Process(void(*FinishFunction)()) // can be called in timer layer or void Function 1 if the time interval is valid 
{
  if(Led.Times)
  {
    if(Led.CountEdgeTime++ >= Led.EdgeTime)
    {
      // turn on/off led
      Led.CountEdgeTime = 0;
      Led.IsOn = !Led.IsOn;
      CFG_LED(Led.IsOn);
      
      // Led at the stage where it always blink
      
      if(Led.Times != 255)
      {
        if(!(--Led.Times) && FinishFunction)
          FinishFunction();
      }
    }
  }
 
}




                 

void Led_Blink(unsigned int Times, unsigned int EdgeTime)
{
  Led.IsOn = 0;
  CFG_LED(0);
  Led.CountEdgeTime = 0xFE;
  Led.EdgeTime = EdgeTime;
  Led.Times = Times * 2;
  
  
}


