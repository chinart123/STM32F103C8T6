#include <stm32f103c8t6.h>
#include <advanced_timer.h>
//test NVIC
void main()
{
  // --- TEST CASE 1: ENABLE EXTI0 (IRQ 6) ---
  // Dự đoán: Tại 0xE000E100, Bit 6 sẽ bật lên 1 (Giá trị tăng thêm 0x40)
  *((unsigned long*)0xE000E100) = (1 << 6); 

  // --- TEST CASE 2: ENABLE TIM4 (IRQ 30) ---
  // Dự đoán: Tại 0xE000E100, Bit 30 sẽ bật lên 1 (Giá trị 0x40000000)
  *((unsigned long*)0xE000E100) = (1 << 30);

//  // --- TEST CASE 3: DISABLE SPI1 (IRQ 35) ---
//  // Dự đoán: Tại 0xE000E184 (ICER1), Bit 3 sẽ bật lên 1 (Giá trị 0x08)
//  *((unsigned long*)0xE000E184) = (1 << 3);

  // --- SỬA LẠI TEST CASE 3: DISABLE SPI1 (IRQ 35) ---
  
  // Bước 3.1: Bật nó lên trước để làm mồi (Ghi vào ISER1 - 0xE000E104)
  // Bấm F10 lần 1: Bạn sẽ thấy CLRENA1 (hoặc SETENA1) hiện 0x00000008
  *((unsigned long*)0xE000E104) = (1 << 3);

  // Bước 3.2: Bây giờ mới thực sự TẮT nó (Ghi vào ICER1 - 0xE000E184)
  // Bấm F10 lần 2: Bạn sẽ thấy 0x00000008 biến mất, quay về 0x00000000.
  *((unsigned long*)0xE000E184) = (1 << 3);
  
  
  // --- TEST CASE 4: ENABLE UART1 (IRQ 37) ---
  // Dự đoán: Tại 0xE000E104 (ISER1), Bit 5 sẽ bật lên 1 (Giá trị 0x20)
  *((unsigned long*)0xE000E104) = (1 << 5);

  // --- TEST CASE 5: DISABLE RTC_ALARM (IRQ 41) ---
  // Dự đoán: Tại 0xE000E184 (ICER1), Bit 9 sẽ bật lên 1 (Giá trị 0x200)
  *((unsigned long*)0xE000E184) = (1 << 9);

  while(1) {}
}