/* Smoke-test No.0 Draft — 4 techniques x 5 gestures on one button (PA0).
 * MODE 1: raw registers | 2: + bit-band | 3: real PWM via LL (LED on PB0!)
 * MODE 4: EXTI button + TIM3 interrupt blink.
 * Gesture engine is shared (TIM2 1 ms polled tick) so techniques compare fairly.
 * Lives in the vendor-track tree (Draft) on purpose: code calling LL must not
 * sit inside C&C++/ (project law NO HAL / NO LL). Guide:
 * "Take note quá trình học thanh ghi/Buoi_12.0_test_package_No.0/Huong_dan_test_No.0.md"
 */
#define MODE 2

#include "stm32f1xx.h"                       /* CMSIS infrastructure     */
#if (MODE >= 3)
#include "stm32f1xx_ll_bus.h"                /* Draft LL from here on    */
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#endif
#if (MODE == 4)
#include "stm32f1xx_ll_exti.h"
#endif

/* ---------- raw infrastructure (all modes) ---------- */
#define REG(a)      (*(volatile unsigned long *)(a))
#define RCC_APB2ENR REG(0x40021018)
#define RCC_APB1ENR REG(0x4002101C)
#define GPIOB_CRH   REG(0x40010C04)
#define GPIOB_IDR   REG(0x40010C08)
#define GPIOB_ODR   REG(0x40010C0C)
#define GPIOC_CRH   REG(0x40011004)
#define GPIOC_ODR   REG(0x4001100C)
#define GPIOC_BSRR  REG(0x40011010)
#define GPIOA_CRL   REG(0x40010800)
#define GPIOA_IDR   REG(0x40010808)
#define GPIOA_ODR   REG(0x4001080C)
#define TIM2_CR1    REG(0x40000000)
#define TIM2_SR     REG(0x40000010)
#define TIM2_PSC    REG(0x40000028)
#define TIM2_ARR    REG(0x4000002C)
#define LED_BB      REG(0x422201B4)          /* bit-band: GPIOC_ODR bit13 */

/* ---- button wiring (USER BOARD: key on PA0). Flip ONE macro if rewired:
 *  BTN_ACTIVE_LOW 1 : button to GND, chip pulls UP   (default)
 *  BTN_ACTIVE_LOW 0 : button to 3V3, chip pulls DOWN                     */
#define BTN_ACTIVE_LOW 1

static volatile unsigned long g_ms = 0;

static void tick_init(void)                  /* TIM2: 1 ms, polled        */
{
    RCC_APB1ENR |= 1UL;                      /* TIM2EN                    */
    TIM2_PSC = 8 - 1;                        /* 8 MHz HSI -> 1 MHz        */
    TIM2_ARR = 1000 - 1;                     /* update every 1 ms         */
    TIM2_CR1 |= 1UL;                         /* CEN                       */
}
static void wait_ms_tick(void)
{
    while (!(TIM2_SR & 1UL)) { }
    TIM2_SR = 0;                             /* clear UIF                 */
    g_ms++;
}

/* ---------- button (PA0) ---------- */
#if (MODE <= 2)
static void btn_init(void)
{
    RCC_APB2ENR |= (1UL << 2);               /* IOPAEN                    */
    GPIOA_CRL = (GPIOA_CRL & ~0xFUL) | 0x8UL;   /* PA0: input PU/PD       */
#if BTN_ACTIVE_LOW
    GPIOA_ODR |= 1UL;                        /* select pull-UP            */
#else
    GPIOA_ODR &= ~1UL;                       /* select pull-DOWN          */
#endif
}
static int btn_raw(void)
{
    int level = (int)(GPIOA_IDR & 1UL);
    return BTN_ACTIVE_LOW ? !level : level;
}
#elif (MODE == 3)
static void btn_init(void)
{
    LL_GPIO_InitTypeDef io;
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    LL_GPIO_StructInit(&io);
    io.Pin = LL_GPIO_PIN_0; io.Mode = LL_GPIO_MODE_INPUT;
    io.Pull = BTN_ACTIVE_LOW ? LL_GPIO_PULL_UP : LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(GPIOA, &io);                /* calls Draft ll_gpio.c     */
}
static int btn_raw(void)
{
    int level = (int)LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
    return BTN_ACTIVE_LOW ? !level : level;
}
#else /* MODE 4: EXTI records the level; polling only reads the mirror    */
static volatile int g_btn_level = 0;
static void btn_init(void)
{
    LL_EXTI_InitTypeDef ex;
    LL_GPIO_InitTypeDef io;
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA
                           | LL_APB2_GRP1_PERIPH_AFIO);
    LL_GPIO_StructInit(&io);
    io.Pin = LL_GPIO_PIN_0; io.Mode = LL_GPIO_MODE_INPUT;
    io.Pull = BTN_ACTIVE_LOW ? LL_GPIO_PULL_UP : LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(GPIOA, &io);
    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTA, LL_GPIO_AF_EXTI_LINE0);
    ex.Line_0_31 = LL_EXTI_LINE_0; ex.LineCommand = ENABLE;
    ex.Mode = LL_EXTI_MODE_IT; ex.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
    LL_EXTI_Init(&ex);                       /* calls Draft ll_exti.c     */
    NVIC_EnableIRQ(EXTI0_IRQn);              /* CMSIS — finding #1        */
}
void EXTI0_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        {
            int level = (int)LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
            g_btn_level = BTN_ACTIVE_LOW ? !level : level;
        }
    }
}
static int btn_raw(void) { return g_btn_level; }
#endif

/* ---------- LED primitive layer (the part under test) ---------- */
#if (MODE == 1)
static void led_init(void)
{
    RCC_APB2ENR |= (1UL << 4);               /* IOPCEN                    */
    GPIOC_CRH = (GPIOC_CRH & ~(0xFUL << 20)) | (0x2UL << 20); /* out 2MHz */
    GPIOC_BSRR = (1UL << 13);                /* off (active-LOW)          */
}
static void led_write(int on)                /* 1 = lit                   */
{ GPIOC_BSRR = on ? (1UL << (13 + 16)) : (1UL << 13); }
static void led_toggle(void) { GPIOC_ODR ^= (1UL << 13); }

#elif (MODE == 2)
static void led_init(void)
{
    RCC_APB2ENR |= (1UL << 4);
    GPIOC_CRH = (GPIOC_CRH & ~(0xFUL << 20)) | (0x2UL << 20);
    LED_BB = 1UL;                            /* off: one bit-band store   */
}
static void led_write(int on) { LED_BB = on ? 0UL : 1UL; }
static void led_toggle(void)  { LED_BB ^= 1UL; }

#elif (MODE == 3)                            /* real PWM: TIM3_CH3 -> PB0 */
static int g_duty = 0;
static void led_init(void)
{
    LL_GPIO_InitTypeDef io;
    LL_TIM_InitTypeDef  t;
    LL_TIM_OC_InitTypeDef oc;
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    LL_GPIO_StructInit(&io);
    io.Pin = LL_GPIO_PIN_0; io.Mode = LL_GPIO_MODE_ALTERNATE;
    io.Speed = LL_GPIO_SPEED_FREQ_LOW; io.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOB, &io);
    LL_TIM_StructInit(&t);
    t.Prescaler = 8 - 1; t.Autoreload = 100 - 1;   /* 10 kHz PWM, duty 0..100 */
    LL_TIM_Init(TIM3, &t);
    LL_TIM_OC_StructInit(&oc);
    oc.OCMode = LL_TIM_OCMODE_PWM1; oc.OCState = LL_TIM_OCSTATE_ENABLE;
    oc.CompareValue = 0;
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH3, &oc);  /* Draft ll_tim.c     */
    LL_TIM_EnableCounter(TIM3);
}
static void led_write(int on)
{ g_duty = on ? 100 : 0; LL_TIM_OC_SetCompareCH3(TIM3, (unsigned)g_duty); }
static void led_toggle(void) { led_write(g_duty == 0); }

#else                                        /* MODE 4: LL GPIO writes    */
static void led_init(void)
{
    LL_GPIO_InitTypeDef io;
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    LL_GPIO_StructInit(&io);
    io.Pin = LL_GPIO_PIN_13; io.Mode = LL_GPIO_MODE_OUTPUT;
    io.Speed = LL_GPIO_SPEED_FREQ_LOW; io.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOC, &io);
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
}
static void led_write(int on)
{ if (on) LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
  else    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13); }
static void led_toggle(void) { LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13); }
#endif

/* ---------- shared behaviors on top of led_write/led_toggle ---------- */
static void fade(int up)                     /* ~2 s, blocking            */
{
    int step;
    for (step = 0; step <= 20; step++) {
        int duty = up ? step : (20 - step);
        unsigned long t_end = g_ms + 100UL;
        while (g_ms < t_end) {
#if (MODE == 3)
            LL_TIM_OC_SetCompareCH3(TIM3, (unsigned)(duty * 5)); /* 0..100 */
#else
            led_write((g_ms % 20UL) < (unsigned long)duty);      /* soft   */
#endif
            wait_ms_tick();
        }
    }
#if (MODE == 3)
    g_duty = up ? 100 : 0;
#endif
}

static int g_blink = 0;                      /* 0 off | 1 even | 2 uneven */
static const unsigned char g_pat[8] = {0u,1u,0u,0u,1u,1u,1u,0u};

#if (MODE == 4)                              /* blink engine = TIM3 ISR   */
static void blink_hw(int arr_ms)
{
    LL_TIM_InitTypeDef t;
    LL_TIM_StructInit(&t);
    t.Prescaler = 8000 - 1;                  /* 1 kHz                     */
    t.Autoreload = (unsigned)arr_ms - 1;
    LL_TIM_Init(TIM3, &t);
    LL_TIM_EnableIT_UPDATE(TIM3);
    NVIC_EnableIRQ(TIM3_IRQn);
    LL_TIM_EnableCounter(TIM3);
}
void TIM3_IRQHandler(void)
{
    static int i = 0;
    LL_TIM_ClearFlag_UPDATE(TIM3);
    if (g_blink == 1) led_toggle();
    else if (g_blink == 2) { led_write(g_pat[i]); i = (i + 1) & 7; }
}
#endif

static void blink_set(int kind)              /* toggle same kind = stop   */
{
    g_blink = (g_blink == kind) ? 0 : kind;
#if (MODE == 4)
    if (g_blink) blink_hw(g_blink == 1 ? 250 : 125);
    else { LL_TIM_DisableCounter(TIM3); LL_TIM_DisableIT_UPDATE(TIM3); }
#endif
    if (!g_blink) led_write(0);
}

static void blink_poll(void)                 /* MODE 1..3: blink in-loop  */
{
#if (MODE != 4)
    static int i = 0;
    static unsigned long t_next = 0;
    if (!g_blink) { t_next = 0; return; }
    if (t_next == 0UL) t_next = g_ms;
    if (g_ms >= t_next) {
        if (g_blink == 1) { led_toggle(); t_next += 250UL; }
        else { led_write(g_pat[i]); i = (i + 1) & 7; t_next += 125UL; }
    }
#endif
}

/* ---------- shared gesture engine (all modes) ---------- */
typedef enum { EV_NONE, EV_C1, EV_C2, EV_C3, EV_H2, EV_H5 } event_t;

static event_t gesture_poll(void)
{
    static int last = 0, clicks = 0;
    static unsigned long t_edge = 0, t_down = 0, t_up = 0;
    event_t ev = EV_NONE;
    int p = btn_raw();
    if (p != last && (g_ms - t_edge) > 30UL) {       /* debounced edge    */
        t_edge = g_ms; last = p;
        if (p) t_down = g_ms;
        else {
            unsigned long held = g_ms - t_down;
            if      (held >= 5000UL) { ev = EV_H5; clicks = 0; }
            else if (held >= 2000UL) { ev = EV_H2; clicks = 0; }
            else { clicks++; t_up = g_ms; }
        }
    }
    if (!last && clicks != 0 && (g_ms - t_up) > 400UL) {  /* window shut  */
        ev = (clicks == 1) ? EV_C1 : (clicks == 2) ? EV_C2 : EV_C3;
        clicks = 0;
    }
    return ev;
}

int main(void)
{
    int k;
    tick_init();
    btn_init();
    led_init();
    for (k = 0; k < 6; k++) {                /* boot heartbeat: 3 blinks  */
        led_write((k & 1) == 0);             /* proves core+tick alive    */
        {
            unsigned long t_end = g_ms + 120UL;
            while (g_ms < t_end) { wait_ms_tick(); }
        }
    }
    led_write(0);
    while (1) {
        event_t ev = gesture_poll();
        if (ev == EV_C1) led_toggle();
        else if (ev == EV_C2) fade(1);
        else if (ev == EV_C3) fade(0);
        else if (ev == EV_H2) blink_set(1);
        else if (ev == EV_H5) blink_set(2);
        blink_poll();
        wait_ms_tick();
    }
}
