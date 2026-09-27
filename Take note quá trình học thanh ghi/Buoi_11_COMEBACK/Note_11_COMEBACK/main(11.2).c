#include <stm32f103c8t6.h>
//copy 1 dữ liệu từ 1 mảng sang 1 mảng khác bằng DMA
//check xem nếu như SỐ LƯỢNG BIT CẦN TRUYỀN DMA_CNDTR vượt quá số data2 thì chuyện gì sẽ xảy ra
/*
@@ nhập "(unsigned char*)data1" "(unsigned char*)data2;20" vào watch để hiển thị ...
*/
unsigned char data1[10] = {1,2,3,4,5,6,7,8,9,10};
unsigned char data2[10] = {0,0,0,0,0,0,0,0,0,0};

void main()
{
  RCC.AHB_ENR.BITS.DMA1 =1;
  DMA1.Channel_1.CMAR = (unsigned long)&data2; //ĐÍCH ĐẾN
  DMA1.Channel_1.CPAR = (unsigned long)&data1; //NGUỒN

  DMA1.Channel_1.CNDTR = 20;  // SỐ LƯỢNG BIT CẦN TRUYỀN
  DMA1.Channel_1.CCR.REG = BIT0 | BIT7 | BIT14;
  
  while (1)
  {
    
  }
  
}