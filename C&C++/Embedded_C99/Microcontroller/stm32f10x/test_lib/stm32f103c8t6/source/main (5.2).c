#include <stm32f103c8t6.h>

void main() {
    // 1. CẤP CLOCK CHO GPIOB VÀ TIM1
    RCC.APB2_ENR.REG |= (1 << 11) | (1 << 3); 

    // 2. CẤU HÌNH PB13 LÀ ALTERNATE FUNCTION PUSH-PULL
    GPIOB.CRH.REG &= ~(0xF << 20); 
    GPIOB.CRH.REG |= (0xB << 20);  

    // 3. CẤU HÌNH TIM1 (Tần số 1KHz, F_clock = 8MHz)
    TIM1.PSC = 7;
    TIM1.ARR = 999; 

    // 4. CHỈNH ĐỘ RỘNG XUNG (PULSE WIDTH = 0.5ms)
    TIM1.CCR1 = 500; 

    // 5. KÍCH HOẠT CHẾ ĐỘ PWM 
    
    // -> ĐÃ SỬA LỖI Ở ĐÂY: Truy cập CCMR1 thông qua REGS[0]
    TIM1.CCMR.OUTPUT.REGS[0] |= (0x6 << 4); // Chế độ PWM Mode 1 cho CH1
    
    // CCER: Cho phép xuất tín hiệu ra kênh bù CH1N (PB13)
    TIM1.CCER.REG |= (1 << 2);

    // BDTR: Bật Main Output Enable (MOE)
    TIM1.BDTR.REG |= (1 << 15);

    // 6. BẬT TIMER (CEN)
    TIM1.CR1.REG |= 1; 

    while(1) {
    }
}