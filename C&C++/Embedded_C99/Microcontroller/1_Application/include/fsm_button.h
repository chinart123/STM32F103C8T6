#ifndef _FSM_BUTTON_H_
#define _FSM_BUTTON_H_

#include <stm32f103c8t6.h>

/*
 * Pin-agnostic push-button driver, split the way the ESP32-Quadcopter does it:
 *
 *   Button_Hardware_Scan(&btn)          -> read the physical pin (+ debounce)
 *   Button_Hardware_Process(&btn, now)  -> run the FSM, publish btn.Event
 *
 * The button ONLY reports what happened. It never measures time itself and
 * never touches the LED. All timing comes from time.c via the 'now' argument
 * (milliseconds). Read btn.Event in main right after Process().
 *
 * Every field is 32-bit on purpose (plenty of flash/RAM; optimise later).
 */

/* Timing, in milliseconds. Override with #define before including if needed. */
#ifndef BTN_DEBOUNCE_MS
#define BTN_DEBOUNCE_MS   15u    /* a press shorter than this is ignored      */
#endif
#ifndef BTN_CLICK_GAP_MS
#define BTN_CLICK_GAP_MS  300u   /* quiet time that ends a click burst        */
#endif

typedef enum
{
    BUTTON_ACTIVE_HIGH = 0,      /* pressed -> pin HIGH                        */
    BUTTON_ACTIVE_LOW  = 1       /* pressed -> pin LOW (button to GND + pull-up) */
} Button_Polarity;

/* What the FSM decided this tick. NONE most of the time. */
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
    /* --- binding (set by Button_Init) --- */
    volatile GPIO_TypeDef* Port;
    unsigned int           Pin;          /* 0..15                             */
    unsigned int           ActiveLow;    /* from Button_Polarity              */
    unsigned int           HoldTargetMs; /* how long to hold before HOLD fires */

    /* --- filled by Hardware_Scan --- */
    unsigned int History;                /* debounce shift register (low 8 bits) */
    unsigned int Pressed;                /* debounced state: 1 = down          */

    /* --- FSM state used by Hardware_Process --- */
    unsigned int WasPressed;             /* Pressed from the previous tick     */
    unsigned int PressStartMs;           /* time (ms) the current press began  */
    unsigned int LastReleaseMs;          /* time (ms) of the last release      */
    unsigned int ClickCount;             /* clicks counted in this burst       */
    unsigned int HoldFired;              /* guard: HOLD fires once per press    */

    /* --- output, read by main --- */
    Button_Event Event;
} Button_TypeDef;

/* Bind a pin as input with the correct pull, set the hold threshold (ms). */
void Button_Init(Button_TypeDef* btn, volatile GPIO_TypeDef* port,
                 unsigned int pin, unsigned int activeLow,
                 unsigned int holdTargetMs);

/* Read the physical pin and debounce it into btn->Pressed. Call every 1 ms. */
void Button_Hardware_Scan(Button_TypeDef* btn);

/* Advance the click/hold FSM using the current time (ms). Sets btn->Event. */
void Button_Hardware_Process(Button_TypeDef* btn, unsigned int now);

#endif /* _FSM_BUTTON_H_ */
