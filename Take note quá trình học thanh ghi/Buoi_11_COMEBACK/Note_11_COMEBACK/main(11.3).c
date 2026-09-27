#include <stm32f103c8t6.h>

// Mảng 20 phần tử để test
unsigned char data1[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
unsigned char data2[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void main()
{
  RCC.AHB_ENR.BITS.DMA1 = 1;
  DMA1.Channel_1.CMAR = (unsigned long)&data2; // ĐÍCH ĐẾN
  DMA1.Channel_1.CPAR = (unsigned long)&data1; // NGUỒN

  // 10 lượt truyền, mỗi lượt dịch chuyển phần cứng 2 byte (16-bit)
  DMA1.Channel_1.CNDTR = 10;  

  // Cấu hình thanh ghi CCR:
  // BIT0: EN (Bật kênh)
  // BIT7: MINC (Tăng địa chỉ bộ nhớ đích)
  // BIT6: PINC (Tăng địa chỉ bộ nhớ nguồn)
  // BIT14: MEM2MEM (Truyền từ RAM sang RAM)
  // BIT8: PSIZE[0] (Chọn 16-bit cho nguồn) -> BIT8 = 1, BIT9 = 0
  // BIT10: MSIZE[0] (Chọn 16-bit cho đích) -> BIT10 = 1, BIT11 = 0
  DMA1.Channel_1.CCR.REG = BIT0 | BIT7 | BIT6 | BIT14 | BIT8 | BIT10;
  
  while (1)
  {
    // Nhập vào Watch: (unsigned char*)data2;20 để kiểm tra kết quả
  }
}
