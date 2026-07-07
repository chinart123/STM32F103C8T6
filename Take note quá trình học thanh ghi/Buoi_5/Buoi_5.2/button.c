#include <button.h>
#include <config.h>


__no_init Button_TypeDef Button;



void Button_Begin() 
{
  Button.Accumulator = 0;
  Button.Count =5;
  Button.Time = 0;
  Button.Status = (BUTTON_STATUS)(READ_BUTTON() + 1);
  Button.LastStatus = BUTTON_STATUS_NONE;
}

void Button_Process()
{
  if(Button.Count--)
    Button.Accumulator += READ_BUTTON() + 1;
  else
  {
    Button.Count = 4;
    Button.Accumulator /= 4;
    if (Button.Accumulator != Button.Status)
    {
      Button.Status = (BUTTON_STATUS)Button.Accumulator;
      Button.LastStatus = Button.Status;
      if (Button.Accumulator == BUTTON_STATUS_FALL)
        Button.Time = 0;
    }
    else
    {
      if(Button.Accumulator == 1)
      {
        if (Button.Time < 255)
          Button.Time++;
      }
    }
    Button.Accumulator = READ_BUTTON() + 1;
  }
    
}
unsigned char Button_Check(BUTTON_STATUS Status, unsigned char Clear)
{
  if (Button.LastStatus == Status)
  {
    if( Clear)
      Button.LastStatus = BUTTON_STATUS_NONE;
      return 1;
  }
  return 0;
}
unsigned char Button_Press()
{
  if (Button.LastStatus == BUTTON_STATUS_RISE && Button.Time < 10)
  {
    Button.LastStatus = BUTTON_STATUS_NONE;
    return 1;
  }
  return 0;
}
unsigned char Button_Hold(unsigned char Time, unsigned char Once)
{
  return 1;
}
unsigned char Button_HoldAccel(ButtonAccel_TypeDef* ButtonAccel)
{
  return 1;
}
void Button_Set(BUTTON_STATUS Status, unsigned char Time)
{
  
}



