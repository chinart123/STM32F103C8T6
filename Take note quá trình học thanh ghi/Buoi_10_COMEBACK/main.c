/**
 * @file    main.c
 * @brief   Chỉ logic. Tầng phần cứng chia ra time / button / led.
 *
 * Vòng lặp chính làm 4 việc, mỗi mili-giây một lần:
 *   1) hỏi fsm_time.c bây giờ là mấy giờ,
 *   2) để nút tự đọc + xử lý (-> btn.Event),
 *   3) chạy LOGIC NGƯỜI DÙNG: mỗi sự kiện nút một 'if',
 *   4) để LED lái chân theo trạng thái mà logic yêu cầu.
 *
 * main không bao giờ đụng thanh ghi GPIO. Muốn đổi chân, chỉ sửa 2 dòng Init.
 * Muốn đổi hành vi, chỉ sửa 4 dòng 'if'.
 *
 *   single click -> bật/tắt
 *   double click -> "sáng dần": sáng lên trong 3 s, lặp lại (PWM mềm)
 *   triple click -> "tối dần":  tối đi trong 3 s, lặp lại (PWM mềm)
 *   hold         -> nháy nháy
 */

#include <stm32f103c8t6.h>
#include "fsm_time.h"
#include "fsm_button.h"
#include "fsm_led.h"

void main(void)
{
    Led_TypeDef    led;
    Button_TypeDef btn;

    Time_Init(8000000u);                                 /* HSI 8 MHz -> 1 ms  */

    /* ---- chọn phần cứng ở đây (port + chân bất kỳ) ---- */
    Led_Init(&led, &GPIOC, 13u, LED_ACTIVE_LOW);         /* LED onboard PC13   */
    Button_Init(&btn, &GPIOA, 0u, BUTTON_ACTIVE_LOW, 800u); /* PA0->GND, giữ 800 ms */

    unsigned int last = Time_Millis();

    while (1)
    {
        unsigned int now = Time_Millis();
        if (now == last) continue;       /* chạy tầng phần cứng mỗi 1 ms       */
        last = now;

        Button_Hardware_Scan(&btn);              /* đọc + chống dội            */
        Button_Hardware_Process(&btn, now);      /* FSM -> btn.Event           */

        /* ---- logic người dùng: mỗi sự kiện một điều kiện, không switch/case ---- */
        if (btn.Event == BTN_EVENT_SINGLE) Led_Set_Toggle(&led);            /* bật/tắt     */
        if (btn.Event == BTN_EVENT_DOUBLE) Led_Set_Fade_Up(&led, 3000u);    /* sáng dần 3s */
        if (btn.Event == BTN_EVENT_TRIPLE) Led_Set_Fade_Down(&led, 3000u);  /* tối dần 3s  */
        if (btn.Event == BTN_EVENT_HOLD)   Led_Set_Blink(&led, 150u, 150u); /* nháy nháy   */

        Led_Hardware_Process(&led, now);         /* lái chân LED               */
    }
}
