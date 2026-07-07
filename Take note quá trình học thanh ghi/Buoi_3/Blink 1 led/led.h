#ifndef _LED_H_
#define _LED_H_


typedef struct
{
  unsigned char IsOn;
  unsigned char Times;
  unsigned char EdgeTime;
  unsigned char CountEdgeTime;
  
  
}Led_TypeDef;

void Led_Begin();
void Led_Process(void(*FinishFunction)()); // can be called in timer layer or void Function 1 if the time interval is valid 
//FinishFunction is used to output an event after what doing next
void Led_Blink(unsigned int Times, unsigned int EdgeTime);



                 
                 
                 
                 
                 
                 
                 
                 
                 
             



#endif