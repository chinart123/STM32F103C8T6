#include <stm32f103c8t6.h>
#include <advanced_timer.h>
//nháy led với tần số 1us
// Định nghĩa địa chỉ NVIC như bài học trước
#define NVIC_ISER0_ADDR   0xE000E100  // Bật ngắt 0-31
#define NVIC_ISER1_ADDR   0xE000E104  // Bật ngắt 32-63
#define NVIC_ISPR1_ADDR   0xE000E204  // Set Pending (Treo ngắt) cho 32-63. Offset là 0x204

// Hàm xử lý ngắt Timer 1 (Ngắt số 25)
// Nhiệm vụ: Đảo trạng thái LED PC13
void TIM1_UP_IRQHandler()
{
  // 1. Xóa cờ ngắt Timer (Bắt buộc)
  TIM1.SR.BITS.UIF = 0; 
  
  // 2. Đảo bit PC13 (XOR với 1)
  // Nếu đang 0 thành 1, đang 1 thành 0 -> LED Nhấp nháy
  GPIOC.ODR.REG ^= (1 << 13); 
}

// Hàm xử lý ngắt UART1 (Ngắt số 37)
// Nhiệm vụ: Đảo trạng thái LED PB12
// Lưu ý: Ta không cần module UART thật, ta sẽ kích hoạt nó bằng "cơm" (phần mềm)
void UART1_IRQHandler()
{
  // 1. Vì là ngắt giả lập bằng phần mềm (Set Pending), 
  // khi nhảy vào đây phần cứng tự xóa cờ Pending NVIC, ta không cần làm gì cả.
  
  // 2. Đảo bit PB12
  GPIOB.ODR.REG ^= (1 << 12);
}

void main()
{
  // --------------------------------------------------------
  // BƯỚC 1: Cấu hình Clock và GPIO cho LED
  // --------------------------------------------------------
  // Bật Clock cho Timer1, GPIOB, GPIOC, GPIOA, AFIO
  RCC.APB2_ENR.REG |= (1 << 11) | (1 << 4) | (1 << 3) | (1 << 2) | (1 << 0);

  // Cấu hình PC13 là Output Push-Pull (Cho TIM1 nháy)
  // PC13 nằm ở CRH (High Register). Mỗi chân chiếm 4 bit.
  // Vị trí: (13 - 8) * 4 = Bit 20.
  GPIOC.CRH.BITS.MODE_13 = 3; // Output 50MHz
  GPIOC.CRH.BITS.CNF_13  = 0; // Push-Pull

  // Cấu hình PB12 là Output Push-Pull (Cho UART1 giả lập nháy)
  GPIOB.CRH.BITS.MODE_12 = 3; 
  GPIOB.CRH.BITS.CNF_12  = 0;

  // --------------------------------------------------------
  // BƯỚC 2: Cấu hình Timer 1 (1ms một lần ngắt)
  // --------------------------------------------------------
  TIM1.PSC.REG = 7;     // Chia tần số cho 8 (8MHz / 8 = 1MHz) -> 1 tick = 1us
  TIM1.ARR.REG = 999;   // Đếm 1000 tick (0-999) -> 1000us = 1ms (Tạo xung nhanh để mắt thấy)
  // *Lưu ý: Để mắt thấy LED nháy chậm, bạn hãy tăng PSC lên 7999 (1ms) và ARR lên 999 (1s)
  
  TIM1.DIER.BITS.UIE = 1;         // Cho phép Timer tạo ngắt Update
  TIM1.CR1.REG = BIT7 | BIT2 | BIT0; // Bật Timer (CEN=1)

  // --------------------------------------------------------
  // BƯỚC 3: Cấu hình NVIC (Dùng kiến thức bạn vừa tính)
  // --------------------------------------------------------
  
  // 3.1 Bật ngắt TIM1_UP (IRQ 25)
  // Nằm ở ISER0 (0xE000E100), Bit 25
  *((volatile unsigned long*)NVIC_ISER0_ADDR) = (1 << 25);

  // 3.2 Bật ngắt UART1 (IRQ 37)
  // Nằm ở ISER1 (0xE000E104), Bit 5 (37 % 32 = 5)
  *((volatile unsigned long*)NVIC_ISER1_ADDR) = (1 << 5);


  while(1)
  {
    // --------------------------------------------------------
    // BƯỚC 4: Kỹ thuật "Software Trigger" (Giả lập ngắt)
    // --------------------------------------------------------
    // Ta muốn test xem ngắt UART1 có chạy không mà không cần cắm dây UART.
    // Ta sẽ dùng thanh ghi ISPR (Interrupt Set-Pending Register).
    // Khi ghi 1 vào bit Pending, NVIC tưởng là ngoại vi đang gọi, nó sẽ nhảy vào hàm ngắt ngay.
    
    // Địa chỉ ISPR1: Base + Offset 0x200 + (Index 1 * 4) = 0xE000E204
    // Set Bit 5 (Tương ứng với UART1)
    
    *((volatile unsigned long*)NVIC_ISPR1_ADDR) = (1 << 5);
    
    // Delay một chút để mắt kịp nhìn thấy LED PB12 đổi màu
    for(volatile int i = 0; i < 1000000; i++); 
  }
}