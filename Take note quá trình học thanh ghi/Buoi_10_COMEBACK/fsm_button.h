#ifndef _FSM_BUTTON_H_
#define _FSM_BUTTON_H_

#include <stm32f103c8t6.h>

/*
 * Driver nút nhấn dùng được cho mọi chân, chia việc y như ESP32-Quadcopter:
 *
 *   Button_Hardware_Scan(&btn)          -> đọc chân vật lý (+ chống dội)
 *   Button_Hardware_Process(&btn, now)  -> chạy FSM, xuất btn.Event
 *
 * Nút CHỈ báo cáo chuyện gì đã xảy ra. Nó không tự đo thời gian và không đụng
 * tới LED. Mọi thời gian đến từ fsm_time.c qua tham số 'now' (mili-giây). Đọc
 * btn.Event trong main ngay sau khi gọi Process().
 *
 * Mọi field cố tình để 32-bit (thừa flash/RAM; tối ưu sau).
 */

/* Thời gian, đơn vị mili-giây. Có thể #define đè trước khi include nếu cần. */
#ifndef BTN_DEBOUNCE_MS
#define BTN_DEBOUNCE_MS   15u    /* nhấn ngắn hơn mức này thì bỏ qua           */
#endif
#ifndef BTN_CLICK_GAP_MS
#define BTN_CLICK_GAP_MS  300u   /* khoảng lặng để kết thúc một chuỗi click    */
#endif

typedef enum
{
    BUTTON_ACTIVE_HIGH = 0,      /* nhấn -> chân mức CAO                       */
    BUTTON_ACTIVE_LOW  = 1       /* nhấn -> chân mức THẤP (nút nối GND + pull-up) */
} Button_Polarity;

/* FSM quyết định gì ở tick này. Đa số thời gian là NONE. */
typedef enum
{
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SINGLE,
    BTN_EVENT_DOUBLE,
    BTN_EVENT_TRIPLE,
    BTN_EVENT_HOLD
} Button_Event;

typedef struct
{
    /* --- gắn phần cứng (đặt bởi Button_Init) --- */
    volatile GPIO_TypeDef* Port;
    unsigned int           Pin;          /* 0..15                             */
    unsigned int           ActiveLow;    /* lấy từ Button_Polarity            */
    unsigned int           HoldTargetMs; /* giữ bao lâu thì phát HOLD          */

    /* --- do Hardware_Scan điền --- */
    unsigned int History;                /* thanh ghi dịch chống dội (8 bit thấp) */
    unsigned int Pressed;                /* trạng thái đã lọc dội: 1 = đang nhấn */

    /* --- trạng thái FSM dùng trong Hardware_Process --- */
    unsigned int WasPressed;             /* Pressed của tick trước            */
    unsigned int PressStartMs;           /* mốc (ms) lần nhấn hiện tại bắt đầu */
    unsigned int LastReleaseMs;          /* mốc (ms) lần nhả gần nhất          */
    unsigned int ClickCount;             /* số click đếm được trong chuỗi này  */
    unsigned int HoldFired;              /* chốt: HOLD chỉ phát 1 lần mỗi lần nhấn */

    /* --- đầu ra, main đọc --- */
    Button_Event Event;
} Button_TypeDef;

/* Gắn 1 chân làm input với pull đúng, đặt ngưỡng giữ (ms). */
void Button_Init(Button_TypeDef* btn, volatile GPIO_TypeDef* port,
                 unsigned int pin, unsigned int activeLow,
                 unsigned int holdTargetMs);

/* Đọc chân vật lý và chống dội thành btn->Pressed. Gọi mỗi 1 ms. */
void Button_Hardware_Scan(Button_TypeDef* btn);

/* Chạy FSM click/giữ theo thời gian hiện tại (ms). Đặt btn->Event. */
void Button_Hardware_Process(Button_TypeDef* btn, unsigned int now);

#endif /* _FSM_BUTTON_H_ */
