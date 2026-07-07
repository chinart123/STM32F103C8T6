/**
 * @file    main.c
 * @brief   Logic only. The hardware layer is split across time / button / led.
 *
 * The main loop does four things, once per millisecond:
 *   1) ask time.c what time it is,
 *   2) let the button read + process itself (-> btn.Event),
 *   3) run the USER LOGIC: one 'if' per button event,
 *   4) let the led drive its pin for whatever state the logic asked for.
 *
 * main never touches a GPIO register. To change pins, edit only the two Init
 * lines. To change behaviour, edit only the four 'if' lines.
 *
 *   single click -> toggle on/off
 *   double click -> "sang dan": fade up over 3 s, looping (soft-PWM)
 *   triple click -> "toi dan":  fade down over 3 s, looping (soft-PWM)
 *   hold         -> blink
 */

#include <stm32f103c8t6.h>
#include "fsm_time.h"
#include "fsm_button.h"
#include "fsm_led.h"

void main(void)
{
    Led_TypeDef    led;
    Button_TypeDef btn;

    Time_Init(8000000u);                                 /* 8 MHz HSI -> 1 ms  */

    /* ---- pick your hardware here (any port + pin) ---- */
    Led_Init(&led, &GPIOC, 13u, LED_ACTIVE_LOW);         /* onboard LED PC13   */
    Button_Init(&btn, &GPIOA, 0u, BUTTON_ACTIVE_LOW, 800u); /* PA0->GND, hold 800 ms */

    unsigned int last = Time_Millis();

    while (1)
    {
        unsigned int now = Time_Millis();
        if (now == last) continue;       /* run the hardware layer every 1 ms  */
        last = now;

        Button_Hardware_Scan(&btn);              /* read + debounce            */
        Button_Hardware_Process(&btn, now);      /* FSM -> btn.Event           */

        /* ---- user logic: one condition per event, no switch/case ---- */
        if (btn.Event == BTN_EVENT_SINGLE) Led_Set_Toggle(&led);            /* on/off      */
        if (btn.Event == BTN_EVENT_DOUBLE) Led_Set_Fade_Up(&led, 3000u);    /* sang dan 3s */
        if (btn.Event == BTN_EVENT_TRIPLE) Led_Set_Fade_Down(&led, 3000u);  /* toi dan 3s  */
        if (btn.Event == BTN_EVENT_HOLD)   Led_Set_Blink(&led, 150u, 150u); /* nhay nhay   */

        Led_Hardware_Process(&led, now);         /* drive the LED pin          */
    }
}
