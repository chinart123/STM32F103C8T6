#include <stm32f103c8t6.h>
//copy 1 dữ liệu từ 1 mảng sang 1 mảng khác bằng DMA
unsigned char data1[10] = {1,2,3,4,5,6,7,8,9,10};
unsigned char data2[10] = {0,0,0,0,0,0,0,0,0,0};

void main()
{
  RCC.AHB_ENR.BITS.DMA1 =1;
  DMA1.Channel_1.CMAR = (unsigned long)&data2;
  DMA1.Channel_1.CPAR = (unsigned long)&data1;
  DMA1.Channel_1.CNDTR = 10;  
  DMA1.Channel_1.CCR.REG = BIT0 | BIT7 | BIT14;
  
  while (1)
  {
    
  }
  
}