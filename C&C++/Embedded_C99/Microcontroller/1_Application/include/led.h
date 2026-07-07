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
void Led_Process(void( *FinishFunction)(unsigned int LedIndex) ); // can be called in timer layer or void Function 1 if the time interval is valid 
/*
@brief
//FinishFunction is a function pointer
//FinishFunction is used to output an event after what doing next
//By default, it will transmit a NULL pointer, because there is only 1 LED, so no need to allocate the address of that LED
//If toggle multiple Led, then FinishFunction() will receive the index of each Led(which is also the address of them
*/
void Led_Blink(unsigned int LedIndex, unsigned int Times, unsigned int EdgeTime);

#endif

