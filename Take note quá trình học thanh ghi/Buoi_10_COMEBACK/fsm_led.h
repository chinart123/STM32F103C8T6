#ifndef _FSM_LED_H_
#define _FSM_LED_H_

#include <stm32f103c8t6.h>

/*
 * Driver LED dùng được cho mọi chân. LED không có input nên không có Scan() —
 * chỉ có:
 *
 *   Led_Set_*(...)                  -> main yêu cầu một trạng thái (toggle/fade/...)
 *   Led_Hardware_Process(&led, now) -> lái chân vật lý theo trạng thái đó
 *
 * Hiệu ứng không chặn và dựa theo thời gian: Process() đọc 'now' (ms, từ
 * fsm_time.c) rồi quyết định mức chân. Không delay(), không tự đếm tick cho LED.
 *
 * Độ sáng (fade) dùng PWM MỀM nên chạy được MỌI chân, kể cả PC13 vốn không có
 * kênh timer. 8 mức ở nhịp 1 ms -> ~125 Hz, không thấy nháy. Không cần thư viện timer.
 *
 * Mọi field cố tình để 32-bit (tối ưu sau).
 */

#define LED_PWM_STEPS   8u          /* độ phân giải PWM mềm (độ sáng 0..8) */

typedef enum
{
    LED_ACTIVE_HIGH = 0,            /* chân CAO -> LED sáng                  */
    LED_ACTIVE_LOW  = 1             /* chân THẤP -> LED sáng (PC13 Blue Pill) */
} Led_Polarity;

typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK,                 /* bật/tắt với thời lượng riêng biệt      */
    LED_MODE_FADE_UP,               /* lặp: tắt -> sáng đầy, rồi lại (PWM mềm) */
    LED_MODE_FADE_DOWN              /* lặp: sáng đầy -> tắt, rồi lại (PWM mềm) */
} Led_Mode;

typedef struct
{
    /* --- gắn phần cứng (đặt bởi Led_Init) --- */
    volatile GPIO_TypeDef* Port;
    unsigned int           Pin;          /* 0..15                            */
    unsigned int           ActiveLow;

    /* --- hành vi hiện tại --- */
    unsigned int Mode;                   /* một trong Led_Mode               */

    /* --- tham số blink --- */
    unsigned int OnMs;
    unsigned int OffMs;
    unsigned int PhaseStartMs;           /* lúc pha bật/tắt hiện tại bắt đầu  */
    unsigned int PhaseOn;                /* 1 = đang ở pha BẬT               */

    /* --- tham số fade (PWM mềm) --- */
    unsigned int Duty;                   /* độ sáng hiện tại 0..LED_PWM_STEPS */
    unsigned int StepMs;                 /* số ms giữa hai lần đổi độ sáng    */
    unsigned int StepStartMs;            /* lúc đổi độ sáng gần nhất          */
} Led_TypeDef;

/* Gắn 1 chân, đặt làm output push-pull 2 MHz, khởi động TẮT. */
void Led_Init(Led_TypeDef* led, volatile GPIO_TypeDef* port,
              unsigned int pin, unsigned int activeLow);

/* Yêu cầu trạng thái từ main (mỗi hàm chỉ đặt Mode + tham số, không chặn). */
void Led_Set_On(Led_TypeDef* led);
void Led_Set_Off(Led_TypeDef* led);
void Led_Set_Toggle(Led_TypeDef* led);
void Led_Set_Blink(Led_TypeDef* led, unsigned int onMs, unsigned int offMs);
/* 'totalMs' là độ dài một lần ramp đầy đủ (vd 3000 = 3 s), rồi lặp lại. */
void Led_Set_Fade_Up(Led_TypeDef* led, unsigned int totalMs);    /* "sáng dần" */
void Led_Set_Fade_Down(Led_TypeDef* led, unsigned int totalMs);  /* "tối dần"  */

/* Lái chân theo mode hiện tại, dùng fsm_time.c. Gọi mỗi 1 ms. */
void Led_Hardware_Process(Led_TypeDef* led, unsigned int now);

#endif /* _FSM_LED_H_ */
