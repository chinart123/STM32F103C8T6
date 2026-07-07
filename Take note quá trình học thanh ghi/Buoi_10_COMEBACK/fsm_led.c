#include "fsm_led.h"

/* ------------------------------------------------------------------ */
/* Hàm GPIO cục bộ — tự chứa, không HAL/LL.                            */
/* ------------------------------------------------------------------ */

static void led_clock(volatile GPIO_TypeDef* port)
{
    if      (port == &GPIOA) RCC.APB2_ENR.BITS.IOPA = 1;
    else if (port == &GPIOB) RCC.APB2_ENR.BITS.IOPB = 1;
    else if (port == &GPIOC) RCC.APB2_ENR.BITS.IOPC = 1;
    else if (port == &GPIOD) RCC.APB2_ENR.BITS.IOPD = 1;
    else if (port == &GPIOE) RCC.APB2_ENR.BITS.IOPE = 1;
}

/* Output push-pull 2 MHz: nibble = (CNF<<2)|MODE = (0<<2)|2 = 0x2. */
static void led_out_2mhz(volatile GPIO_TypeDef* port, unsigned int pin)
{
    unsigned int shift = (pin < 8u ? pin : (pin - 8u)) * 4u;
    if (pin < 8u)
    {
        port->CRL.REG &= ~(0xFUL << shift);
        port->CRL.REG |=  (0x2UL << shift);
    }
    else
    {
        port->CRH.REG &= ~(0xFUL << shift);
        port->CRH.REG |=  (0x2UL << shift);
    }
}

/* Lái chân từ cờ logic "sáng?" (BSRR/BRR ghi 1 bit nguyên tử, STANDARD.md 2). */
static void led_pin(Led_TypeDef* led, unsigned int lit)
{
    unsigned int high = led->ActiveLow ? (!lit) : lit;
    if (high) led->Port->BSRR.REG = (1UL << led->Pin);   /* set   */
    else      led->Port->BRR.REG  = (1UL << led->Pin);   /* reset */
}

/* PWM mềm: trong cửa sổ 8 ms, giữ LED sáng trong 'Duty' của 8 ms. */
static void led_pwm(Led_TypeDef* led, unsigned int now)
{
    led_pin(led, (now % LED_PWM_STEPS) < led->Duty);
}

/* ------------------------------------------------------------------ */
/* API công khai                                                       */
/* ------------------------------------------------------------------ */

void Led_Init(Led_TypeDef* led, volatile GPIO_TypeDef* port,
              unsigned int pin, unsigned int activeLow)
{
    led->Port      = port;
    led->Pin       = pin;
    led->ActiveLow = activeLow;

    led->Mode         = LED_MODE_OFF;
    led->OnMs         = 0u;
    led->OffMs        = 0u;
    led->PhaseStartMs = 0u;
    led->PhaseOn      = 0u;
    led->Duty         = 0u;
    led->StepMs       = 0u;
    led->StepStartMs  = 0u;

    led_clock(port);
    led_out_2mhz(port, pin);
    led_pin(led, 0u);              /* khởi động TẮT */
}

void Led_Set_On(Led_TypeDef* led)
{
    led->Mode = LED_MODE_ON;
}

void Led_Set_Off(Led_TypeDef* led)
{
    led->Mode = LED_MODE_OFF;
}

void Led_Set_Toggle(Led_TypeDef* led)
{
    /* toggle chỉ có nghĩa giữa ON và OFF thuần */
    led->Mode = (led->Mode == LED_MODE_ON) ? LED_MODE_OFF : LED_MODE_ON;
}

void Led_Set_Blink(Led_TypeDef* led, unsigned int onMs, unsigned int offMs)
{
    led->Mode         = LED_MODE_BLINK;
    led->OnMs         = onMs;
    led->OffMs        = offMs;
    led->PhaseOn      = 1u;
    led->PhaseStartMs = 0u;       /* Process sẽ bắt đầu pha ở lần gọi đầu */
}

/* Sáng dần rồi lặp: bắt đầu tối, sáng lên trong totalMs, rồi quay về tối. */
void Led_Set_Fade_Up(Led_TypeDef* led, unsigned int totalMs)
{
    led->Mode        = LED_MODE_FADE_UP;
    led->Duty        = 0u;                          /* bắt đầu TẮT        */
    led->StepMs      = totalMs / LED_PWM_STEPS;     /* thời gian mỗi +1 mức */
    led->StepStartMs = 0u;
}

/* Tối dần rồi lặp: bắt đầu sáng, tối đi trong totalMs, rồi quay về sáng. */
void Led_Set_Fade_Down(Led_TypeDef* led, unsigned int totalMs)
{
    led->Mode        = LED_MODE_FADE_DOWN;
    led->Duty        = LED_PWM_STEPS;               /* bắt đầu SÁNG đầy    */
    led->StepMs      = totalMs / LED_PWM_STEPS;     /* thời gian mỗi -1 mức */
    led->StepStartMs = 0u;
}

/* Mỗi mode một nhánh. Không switch — mỗi 'if' là một hành vi LED rõ ràng. */
void Led_Hardware_Process(Led_TypeDef* led, unsigned int now)
{
    if (led->Mode == LED_MODE_OFF)
    {
        led_pin(led, 0u);
    }

    if (led->Mode == LED_MODE_ON)
    {
        led_pin(led, 1u);
    }

    if (led->Mode == LED_MODE_BLINK)
    {
        unsigned int limit = led->PhaseOn ? led->OnMs : led->OffMs;
        if (led->PhaseStartMs == 0u) led->PhaseStartMs = now;   /* lần gọi đầu */

        if ((now - led->PhaseStartMs) >= limit)
        {
            led->PhaseOn      = !led->PhaseOn;
            led->PhaseStartMs = now;
        }
        led_pin(led, led->PhaseOn);
    }

    if (led->Mode == LED_MODE_FADE_UP)
    {
        if (led->StepStartMs == 0u) led->StepStartMs = now;

        if ((now - led->StepStartMs) >= led->StepMs)
        {
            led->StepStartMs = now;
            if (led->Duty >= LED_PWM_STEPS) led->Duty = 0u;   /* đầy -> quay lại tắt */
            else                            led->Duty++;
        }
        led_pwm(led, now);
    }

    if (led->Mode == LED_MODE_FADE_DOWN)
    {
        if (led->StepStartMs == 0u) led->StepStartMs = now;

        if ((now - led->StepStartMs) >= led->StepMs)
        {
            led->StepStartMs = now;
            if (led->Duty == 0u) led->Duty = LED_PWM_STEPS;   /* tắt -> quay lại đầy */
            else                 led->Duty--;
        }
        led_pwm(led, now);
    }
}
