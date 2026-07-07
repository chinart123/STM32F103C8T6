#include "fsm_button.h"

/* ------------------------------------------------------------------ */
/* Hàm GPIO cục bộ — tự chứa, không HAL/LL.                            */
/* ------------------------------------------------------------------ */

static void btn_clock(volatile GPIO_TypeDef* port)
{
    if      (port == &GPIOA) RCC.APB2_ENR.BITS.IOPA = 1;
    else if (port == &GPIOB) RCC.APB2_ENR.BITS.IOPB = 1;
    else if (port == &GPIOC) RCC.APB2_ENR.BITS.IOPC = 1;
    else if (port == &GPIOD) RCC.APB2_ENR.BITS.IOPD = 1;
    else if (port == &GPIOE) RCC.APB2_ENR.BITS.IOPE = 1;
}

/* Input có pull: nibble = (CNF<<2)|MODE = (2<<2)|0 = 0x8.
   Rồi ODR chọn hướng: 1 = pull-up, 0 = pull-down. */
static void btn_input_pull(volatile GPIO_TypeDef* port, unsigned int pin,
                           unsigned int pullUp)
{
    unsigned int shift = (pin < 8u ? pin : (pin - 8u)) * 4u;
    if (pin < 8u)
    {
        port->CRL.REG &= ~(0xFUL << shift);
        port->CRL.REG |=  (0x8UL << shift);
    }
    else
    {
        port->CRH.REG &= ~(0xFUL << shift);
        port->CRH.REG |=  (0x8UL << shift);
    }

    if (pullUp) port->BSRR.REG = (1UL << pin);   /* ODR=1 -> pull-up   */
    else        port->BRR.REG  = (1UL << pin);   /* ODR=0 -> pull-down */
}

/* Đọc chân, đổi sang "đang nhấn?" theo logic (1 = đang nhấn). */
static unsigned int btn_raw_pressed(Button_TypeDef* btn)
{
    unsigned int level = (btn->Port->IDR.REG >> btn->Pin) & 1u;
    return btn->ActiveLow ? (level == 0u) : (level == 1u);
}

/* ------------------------------------------------------------------ */
/* API công khai                                                       */
/* ------------------------------------------------------------------ */

void Button_Init(Button_TypeDef* btn, volatile GPIO_TypeDef* port,
                 unsigned int pin, unsigned int activeLow,
                 unsigned int holdTargetMs)
{
    btn->Port         = port;
    btn->Pin          = pin;
    btn->ActiveLow    = activeLow;
    btn->HoldTargetMs = holdTargetMs;

    btn->History       = 0u;
    btn->Pressed       = 0u;
    btn->WasPressed    = 0u;
    btn->PressStartMs  = 0u;
    btn->LastReleaseMs = 0u;
    btn->ClickCount    = 0u;
    btn->HoldFired     = 0u;
    btn->Event         = BTN_EVENT_NONE;

    btn_clock(port);
    /* active-low nghỉ ở mức CAO -> pull-up; active-high nghỉ ở THẤP -> pull-down. */
    btn_input_pull(port, pin, activeLow ? 1u : 0u);
}

/* Chỉ đọc + chống dội. Không thời gian, không FSM — chỉ là sự thật về chân. */
void Button_Hardware_Scan(Button_TypeDef* btn)
{
    unsigned int raw = btn_raw_pressed(btn);

    btn->History = ((btn->History << 1) | raw) & 0xFFu;   /* giữ 8 mẫu       */
    if      (btn->History == 0xFFu) btn->Pressed = 1u;    /* 8 mẫu liền = nhấn */
    else if (btn->History == 0x00u) btn->Pressed = 0u;    /* 8 mẫu liền = nhả  */
}

/* Chỉ FSM. Dùng btn->Pressed và 'now' (ms); tạo ra btn->Event. */
void Button_Hardware_Process(Button_TypeDef* btn, unsigned int now)
{
    btn->Event = BTN_EVENT_NONE;

    /* cạnh nhấn: nhớ lúc bắt đầu, nạp lại chốt hold. */
    if (btn->Pressed && !btn->WasPressed)
    {
        btn->PressStartMs = now;
        btn->HoldFired    = 0u;
    }

    /* giữ đủ lâu -> HOLD một lần; một lần giữ không tính là click. */
    if (btn->Pressed && !btn->HoldFired &&
        (now - btn->PressStartMs) >= btn->HoldTargetMs)
    {
        btn->Event      = BTN_EVENT_HOLD;
        btn->HoldFired  = 1u;
        btn->ClickCount = 0u;
    }

    /* cạnh nhả: một lần nhấn thật (đã lọc dội, không phải giữ) = một click. */
    if (!btn->Pressed && btn->WasPressed && !btn->HoldFired)
    {
        if ((now - btn->PressStartMs) >= BTN_DEBOUNCE_MS)
        {
            btn->ClickCount++;
            btn->LastReleaseMs = now;
        }
    }

    /* chuỗi kết thúc sau một khoảng lặng -> phát sự kiện theo số click. */
    if (!btn->Pressed && btn->ClickCount > 0u &&
        (now - btn->LastReleaseMs) >= BTN_CLICK_GAP_MS)
    {
        if      (btn->ClickCount == 1u) btn->Event = BTN_EVENT_SINGLE;
        else if (btn->ClickCount == 2u) btn->Event = BTN_EVENT_DOUBLE;
        else if (btn->ClickCount == 3u) btn->Event = BTN_EVENT_TRIPLE;
        /* 4 click trở lên: bỏ qua */
        btn->ClickCount = 0u;
    }

    btn->WasPressed = btn->Pressed;
}
