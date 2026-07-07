#include <led.h>
#include <config.h>

__no_init Led_TypeDef Led[NUMBER_OF_LED] ;




void Led_Begin()
{
  for( unsigned int i = 0; i <NUMBER_OF_LED; i++)
  {
    Led[i].IsOn = 0;
    CFG_LED(i,0);
    Led[i].Times = 0;
    Led[i].EdgeTime = 0;
    Led[i].CountEdgeTime = 0;
  }
}

//FinishFunction is used to output an event after what doing next
void Led_Process(void( *FinishFunction)() ) // can be called in timer layer or void Function 1 if the time interval is valid 
{
  for( unsigned int i = 0; i < NUMBER_OF_LED; i++)
      {
        if(Led[i].Times)
        {
          if(Led[i].CountEdgeTime++ >= Led[i].EdgeTime)
          {
            Led[i].CountEdgeTime = 0;
            Led[i].IsOn = !Led[i].IsOn;
            CFG_LED(i,Led[i].IsOn);
            if(Led[i].Times != 255)
            {
              if(!(--Led[i].Times) && FinishFunction)
                FinishFunction();
            }
          }
        }
      }
}
      

void Led_Blink(unsigned int LedIndex,unsigned int Times, unsigned int EdgeTime)
{
  Led[LedIndex].IsOn = 0;
  CFG_LED(LedIndex,0);
  Led[LedIndex].CountEdgeTime = 0xFE;
  Led[LedIndex].EdgeTime = EdgeTime;
  Led[LedIndex].Times = Times * 2;
  
  
}


