#ifndef _FSM_LED_H_
#define _FSM_LED_H_

#include <stm32f103c8t6.h>

/*
 * Pin-agnostic LED driver. The LED has no input, so it has no Scan() — only:
 *
 *   Led_Set_*(...)                 -> main asks for a state (toggle/fade/...)
 *   Led_Hardware_Process(&led, now)-> drive the physical pin for this state
 *
 * Effects are non-blocking and time-based: Process() reads 'now' (ms, from
 * time.c) and decides the pin level. No delay(), no per-LED tick counting.
 *
 * Brightness (fade / breathe) uses SOFTWARE PWM so it works on ANY pin,
 * including PC13 which has no hardware timer channel. 8 levels at a 1 ms
 * process cadence -> ~125 Hz, flicker-free. No timer library needed.
 *
 * Every field is 32-bit on purpose (optimise later).
 */

#define LED_PWM_STEPS   8u          /* soft-PWM resolution (brightness 0..8) */

typedef enum
{
    LED_ACTIVE_HIGH = 0,            /* pin HIGH -> LED on                    */
    LED_ACTIVE_LOW  = 1             /* pin LOW  -> LED on (Blue Pill PC13)   */
} Led_Polarity;

typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK,                 /* on/off with independent durations      */
    LED_MODE_FADE_UP,               /* loop: off -> full, restart (soft-PWM)  */
    LED_MODE_FADE_DOWN              /* loop: full -> off, restart (soft-PWM)  */
} Led_Mode;

typedef struct
{
    /* --- binding (set by Led_Init) --- */
    volatile GPIO_TypeDef* Port;
    unsigned int           Pin;          /* 0..15                            */
    unsigned int           ActiveLow;

    /* --- current behaviour --- */
    unsigned int Mode;                   /* one of Led_Mode                  */

    /* --- blink params --- */
    unsigned int OnMs;
    unsigned int OffMs;
    unsigned int PhaseStartMs;           /* when the current on/off phase began */
    unsigned int PhaseOn;                /* 1 = we are in the ON phase        */

    /* --- fade params (soft-PWM) --- */
    unsigned int Duty;                   /* current brightness 0..LED_PWM_STEPS */
    unsigned int StepMs;                 /* ms between brightness steps        */
    unsigned int StepStartMs;            /* when the last brightness step ran  */
} Led_TypeDef;

/* Bind a pin, drive it as a 2 MHz push-pull output, start OFF. */
void Led_Init(Led_TypeDef* led, volatile GPIO_TypeDef* port,
              unsigned int pin, unsigned int activeLow);

/* State requests from main (each just sets Mode + params, no blocking). */
void Led_Set_On(Led_TypeDef* led);
void Led_Set_Off(Led_TypeDef* led);
void Led_Set_Toggle(Led_TypeDef* led);
void Led_Set_Blink(Led_TypeDef* led, unsigned int onMs, unsigned int offMs);
/* 'totalMs' is the length of one full ramp (e.g. 3000 = 3 s), then it loops. */
void Led_Set_Fade_Up(Led_TypeDef* led, unsigned int totalMs);    /* "sang dan" */
void Led_Set_Fade_Down(Led_TypeDef* led, unsigned int totalMs);  /* "toi dan"  */

/* Drive the pin for the current mode using time.c. Call every 1 ms. */
void Led_Hardware_Process(Led_TypeDef* led, unsigned int now);

#endif /* _FSM_LED_H_ */
