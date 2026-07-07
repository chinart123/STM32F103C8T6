#ifndef _FSM_TIME_H_
#define _FSM_TIME_H_

#include <stm32f103c8t6.h>

/*
 * Bộ đếm thời gian mili-giây dựa trên SysTick (chạy bằng ngắt).
 *
 *   Time_Init(8000000u);          // lõi 8 MHz -> 1 tick = 1 ms
 *   unsigned int now = Time_Millis();
 *   Time_Delay(500u);             // chặn (blocking), chỉ dùng lúc khởi tạo
 *
 * SysTick ngắt mỗi 1 ms và cộng dồn một bộ đếm chạy tự do, nhờ đó mọi module
 * khác (button, led) chỉ cần hỏi "mấy giờ rồi?" thay vì tự đếm tick. Đây là nơi
 * DUY NHẤT sở hữu thời gian.
 *
 * LƯU Ý: file này định nghĩa SysTick_Handler(). Startup của IAR đã khai báo nó
 * dạng weak nên hàm của ta tự đè lên. Đừng định nghĩa SysTick_Handler lần thứ hai
 * ở bất kỳ đâu.
 */

/* Cấu hình SysTick cho 1 tick = 1 ms theo tần số lõi (Hz) rồi bắt đầu đếm.
   Bật ngắt SysTick (không cần cấu hình NVIC). */
void         Time_Init(unsigned int coreHz);

/* Số mili-giây đã trôi kể từ Time_Init(). Tràn sau ~49 ngày. */
unsigned int Time_Millis(void);

/* Chờ bận 'ms' mili-giây. Chỉ dùng lúc khởi tạo, tuyệt đối không trong vòng lặp chính. */
void         Time_Delay(unsigned int ms);

#endif /* _FSM_TIME_H_ */
