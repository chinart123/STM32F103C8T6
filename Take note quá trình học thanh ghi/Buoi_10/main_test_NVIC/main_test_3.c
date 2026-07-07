#include <stm32f103c8t6.h>
#include <advanced_timer.h>
// Định nghĩa địa chỉ NVIC
#define NVIC_ISER0   0xE000E100
#define NVIC_ISER1   0xE000E104
#define NVIC_ICER1   0xE000E184

void main()
{
  // ==========================================================
  // BƯỚC CHUẨN BỊ: CẤP CLOCK CHO CÁC NGOẠI VI
  // (Bắt buộc phải cấp clock thì mới ghi được vào thanh ghi con)
  // ==========================================================
  
  // 1. Bật Clock cho AFIO (cho EXTI), GPIOA, SPI1, USART1 (APB2)
  RCC.APB2_ENR.REG |= (1 << 0) | (1 << 2) | (1 << 12) | (1 << 14);

  // 2. Bật Clock cho TIM4, PWR, BKP (cho RTC) (APB1)
  RCC.APB1_ENR.REG |= (1 << 2) | (1 << 27) | (1 << 28);
  
  // 3. Mở khóa cho phép ghi vào RTC (Bắt buộc với RTC)
  PWR.CR.REG |= (1 << 8); // Bit DBP: Disable Backup Domain Protection

  // ==========================================================
  // TEST 1: EXTI0 (Ngắt ngoài Line 0)
  // ==========================================================
  // [Cầu dao tổng]: NVIC ISER0 - Bit 6
  *((volatile unsigned long*)NVIC_ISER0) = (1 << 6); 
  
  // [Cầu dao con]: EXTI_IMR - Bit 0 (Interrupt Mask Register)
  EXTI.IMR.REG |= (1 << 0);


  // ==========================================================
  // TEST 2: TIM4 (Ngắt Timer 4)
  // ==========================================================
  // [Cầu dao tổng]: NVIC ISER0 - Bit 30
  *((volatile unsigned long*)NVIC_ISER0) = (1 << 30);
  
  // [Cầu dao con]: TIM4_DIER - Bit 0 (Update Interrupt Enable)
  TIM4.DIER.REG |= (1 << 0);


  // ==========================================================
  // TEST 3: SPI1 (Ngắt SPI1) - Kịch bản: Bật rồi Tắt
  // ==========================================================
  // [Cầu dao tổng - BẬT]: NVIC ISER1 - Bit 3
  *((volatile unsigned long*)NVIC_ISER1) = (1 << 3);
  
  // [Cầu dao con]: SPI1_CR2 - Bit 7 (TXEIE - Tx Buffer Empty Interrupt)
  SPI1.CR2.REG |= (1 << 7);
  
  // [Cầu dao tổng - TẮT]: NVIC ICER1 - Bit 3
  *((volatile unsigned long*)NVIC_ICER1) = (1 << 3);


  // ==========================================================
  // TEST 4: UART1 (Ngắt UART1)
  // ==========================================================
  // [Cầu dao tổng]: NVIC ISER1 - Bit 5
  *((volatile unsigned long*)NVIC_ISER1) = (1 << 5);
  
  // [Cầu dao con]: USART1_CR1 - Bit 5 (RXNEIE - Read Data Register Not Empty)
  UART1.CR1.REG |= (1 << 5);


  // ==========================================================
  // TEST 5: RTC_ALARM (Ngắt Báo thức thời gian thực)
  // ==========================================================
  // [Cầu dao tổng - TẮT thử]: NVIC ICER1 - Bit 9
  *((volatile unsigned long*)NVIC_ICER1) = (1 << 9);

  // [Cầu dao con]: RTC_CRH - Bit 1 (ALRIE - Alarm Interrupt Enable)
  // Lưu ý: RTC ghi rất chậm, khi debug có thể phải F10 vài lần mới thấy cập nhật
  RTC.CRH.REG |= (1 << 1);

  while(1) {}
}