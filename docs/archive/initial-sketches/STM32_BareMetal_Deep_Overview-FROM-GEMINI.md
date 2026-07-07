# STM32F103C8T6 Bare-Metal Deep Overview

## 1. Introduction & Attribution
**ATTRIBUTION:** The source code and hardware register mapping techniques analyzed in this document were originally authored and architected by YouTube creator **[@lies9823](https://www.youtube.com/@lies9823)**. This incredible bare-metal C codebase maps out the entire STM32F103C8T6 hardware register map entirely from scratch. All credit for the underlying code, architectural patterns (unions/structs for registers), and learning progression belongs to him.

This document provides a deep, register-level analysis of that progression across 10 distinct learning sessions (Buoi_1 to Buoi_10), moving from direct memory address manipulation to sophisticated abstractions like macro-based register access, bit-banding, and interrupt-driven state machines.

## 2. Codebase Architecture & Progression

### Sessions 1-3: Register Structures and Basic Abstractions
- **Initial Setup:** The codebase starts with direct address assignment using compiler-specific directives (`__root __no_init GPIO_TypeDef GPIOB @ 0x40010C00;`).
- **Data Structures:** Extensive use of C `struct` and `union` paradigms to map out the 32-bit registers (e.g., `CRL`, `CRH`, `ODR`, `IDR`, `BSRR`). BITS and REG unions enable intuitive manipulation of single bits or entire 32-bit words.
- **Macros:** `BUNION` and `__VA_ARGS__` macros are introduced in `rcc.h` and `define.h` to minimize repetitive struct definitions and keep `main.c` clean.

### Sessions 4-5: Standard C and Peripheral Modularization
- **Transition to Standard C:** Replacing compiler-specific memory assignments with standard pointers (`#define GPIOB (*(volatile GPIO_TypeDef* )0x40010C00)`). A startup script (`startup_stm32f10x_md.s`) is integrated.
- **Modularization:** Creation of isolated peripheral modules (`gpio.c/h`, `led.c/h`, `button.c/h`).
- **State Handling:** Transitioning from primitive `for`-loop delays to structured state processing for buttons, introducing concepts like Press, Rise, Fall, Hold, and acceleration (`ButtonAccel_TypeDef`).

### Sessions 6-7: Bit-banding and System Clock Optimization
- **Bit-Banding:** `bitband.h` introduces Cortex-M3 bit-banding, mapping bit aliases to base regions, allowing atomic read-modify-write operations on peripheral registers without race conditions (e.g., `GPIOB_BITBAND.ODR.b12 = 1;`).
- **Clock Tree Configuration:** Focuses on configuring Flash memory latency to match high clock speeds. The MCO (Microcontroller Clock Output) pin (PA8) is activated to verify the System Clock (SYSCLK) using a Logic Analyzer.

### Sessions 8-9: System Timer (SysTick) and State Machines
- **SysTick Integration:** Moved away from blocking loops to precise time-keeping using `STK_Init()`. Used to generate exact delays (e.g., 25µs, 1ms) for Servo motor PWM generation and LED blinking.
- **IR Signal Capture:** Implemented a state machine (`IR_STATE_IDLE`, `IR_STATE_WAIT_DATA_LOW`, etc.) inside the `SysTick_Handler` to capture Infrared (IR) break time signals by sampling GPIO states.

### Session 10: Advanced Timers and NVIC
- **NVIC Module:** Configured the Nested Vectored Interrupt Controller to handle hardware interrupts asynchronously, abandoning polling. (e.g., Setting `BIT25` in `ISER0` for `TIM1_UP_IRQHandler`).
- **Advanced Timers (TIM1):** Introduced `advanced_timer.h` for configuring TIM1 (PWM generation on PA8). Included complex struct-union mapping for registers like `CCER`, `BDTR`, `CCMR`, ensuring accurate bit-level configurations.

## 3. Peripheral Configurations

### GPIO (General Purpose Input/Output)
Configured using `CRL` and `CRH` for mode and configuration bits. Custom Enums (`GPIO_MODE_OUTPUT_PUSHPULL_10MHz`, `GPIO_MODE_INPUT_PULL`) map exactly to the bit-pairs defined by ST's reference manual. State flipping leverages `ODR`, while `BSRR` / `BRR` are used for atomic set/reset without bit-banding.

### RCC (Reset and Clock Control)
Implemented with highly detailed bit-fields (e.g., `RCC.APB2_ENR.BITS.IOPB = 1`). Control evolved from simple GPIO clock enables to mapping out Advanced Timer (`TIM1`) and Alternative Function I/O (`AFIO`) clocks.

### NVIC & EXTI
NVIC configuration is done via direct memory access to the `ISER0` register (`*((unsigned long*)0xE000E100) = BIT25;` to enable TIM1 UP interrupt). EXTI and general interrupts rely heavily on properly clearing the Status Register (SR) flags within the IRQ Handler (`TIM1.SR.REG = 0;`) to avoid infinite interrupt loops.

### Timers (SysTick & Advanced TIM1)
- **SysTick:** Configured explicitly in `cortex_m3.c/h` (`STK_Init()`). Drives finite state machines (like IR capture) by counting periods without data line changes.
- **TIM1:** Complete mapping of advanced features such as dead-time generation (`BDTR`), capture/compare (`CCR1`, `CCER`), and auto-reload preload (`ARR`, `PSC`). Generates high-frequency PWM for specific peripheral control (e.g., servo motors, signaling).

## 4. Key Takeaways for AI Agents
- **Struct/Union Mastery:** The codebase relies uniquely on nested anonymous structs and unions to seamlessly bridge C-level logic with hardware registers.
- **Hardware-Level Abstraction:** Avoids HAL/LL libraries completely, offering raw insights into Cortex-M3 memory spaces, atomic operations (Bit-banding), and precise clock controls.
- **Stateful Event Handling:** Non-blocking design patterns are heavily favored in later sessions (SysTick state machines for signal capture), serving as an ideal template for robust embedded systems.

## 5. Project Directory Tree
This is the directory structure for the `Take note quá trình học thanh ghi` folder containing the entire STM32 bare-metal progression:

```text
D:\LIBRARIES\TAKE NOTE QUá TRìNH HọC THANH GHI
+---Buoi_1
|       define.h
|       main.c
|       rcc.h
|       Takenote_buoi_1.txt
|       
+---Buoi_10
|   |   advanced_timer.h
|   |   gpio.h
|   |   main(10.1).txt
|   |   main(10.2).txt
|   |   main(10.3).txt
|   |   Note_Buoi_10.txt
|   |   startup_stm32f103cx.s
|   |   
|   +---learn some syntax stuff
|   |       main(10.1).txt
|   |       main(10.1_sketch).txt
|   |       
|   \---main_test_NVIC
|           main_test_1.c
|           main_test_2.c
|           main_test_3.c
|           main_test_4.txt
|           main_test_5.txt
|           main_test_i2c_1.txt
|           main_test_i2c_2.txt
|           main_test_i2c_3.txt
|           main_test_i2c_4.txt
|           Note_Buoi_10_Tu_hoc.txt
|           Note_test_i2c.txt
|           Note_test_lcd(i2c)_button(PB13)_led(TIMER).txt
|           test_lcd(i2c)_button(PB13)_led(TIMER).txt
|           
+---Buoi_2
|       common.h
|       define.h
|       main (1).c
|       rcc.h
|       regulation.c
|       takenote_Buoi_2.txt
|       
+---Buoi_3
|   +---Blink 1 led
|   |       common.h
|   |       config.h
|   |       gpio.c
|   |       gpio.h
|   |       led.c
|   |       led.h
|   |       main (2).c
|   |       rcc.h
|   |       Takenote_buoi_3.1.txt
|   |       what_change.txt
|   |       
|   \---Blink n led
|           common.h
|           config.h
|           define.h
|           gpio.c
|           gpio.h
|           led.c
|           main (3).c
|           rcc.h
|           what_change.txt
|           
+---Buoi_4
|       Note.txt
|       
+---Buoi_5
|   +---Buoi_5.1
|   |       button.c
|   |       button.h
|   |       config.h
|   |       gpio.c
|   |       gpio.h
|   |       led.c
|   |       led.h
|   |       main (5.1).c
|   |       Note.txt
|   |       rcc.h
|   |       
|   +---Buoi_5.2
|   |       button.c
|   |       main (5.2).c
|   |       
|   +---Buoi_5.3
|   |       button.c
|   |       main (5.3).c
|   |       
|   +---Buoi_5.4
|   |       button.c
|   |       main (5.4).c
|   |       
|   \---Buoi_5.5
|           button.c
|           button.h
|           main (5.5).c
|           
+---Buoi_6
|       bitband.h
|       gpio.h
|       main (6).c
|       Note.txt
|       
+---Buoi_7
|   |   eic320100.html.url
|   |   main(7.1).txt
|   |   main(7.2).txt
|   |   main(debug_v1).txt
|   |   main(debug_v2).txt
|   |   Note.txt
|   |   
|   \---Helper file
|           bitband.h
|           cortex_m3.h
|           flash.c
|           flash.h
|           gpio.c
|           gpio.h
|           rcc.h
|           startup_stm32f103cx.s
|           
+---Buoi_8
|   |   main(8.1).txt
|   |   main(8.2).txt
|   |   main(8.3).txt
|   |   main(8.4).txt
|   |   main(8.5).txt
|   |   Note.txt
|   |   
|   \---Helper file
|           bitband.h
|           cortex_m3.h
|           flash.c
|           flash.h
|           gpio.c
|           gpio.h
|           rcc.h
|           startup_stm32f103cx.s
|           
\---Buoi_9
    |   main(9.1).c.txt
    |   main(9.2).c.txt
    |   main(9.3).c.txt
    |   Note_Buoi_9.txt
    |   
    \---Helper_files
        +---driver
        |   |   gpio.c
        |   |   
        |   \---include
        |           base_timer.h
        |           basic_timer.h
        |           bitband.h
        |           flash.c
        |           flash.h
        |           general_timer.h
        |           gpio.h
        |           rcc.h
        |           
        \---stm32f103c8t6
            \---test_lib
                +---include
                |       config.h
                |       cortex_m3.h
                |       stm32f103c8t6.h
                |       
                \---source
                        cortex_m3.c
                        main (5.2).c
                        startup_stm32f103cx.s
```

## 6. High-Level Project Directory
This is the macro-level directory structure for the entire workspace (`D:\libraries`), including the architecture guides and the `C&C++` codebase:

```text
D:\LIBRARIES
+---C&C++
|   \---Embedded_C99
|       \---Microcontroller
|           +---0_common
|           |   \---include
|           +---1_Application
|           |   +---button
|           |   +---include
|           |   \---led
|           \---stm32f10x
|               +---driver
|               |   \---include
|               \---test_lib
|                   +---DebugConfig
|                   +---RTE
|                   |   \---Device
|                   |       \---STM32F103C8
|                   +---stm32f103c8t6
|                   |   +---Debug
|                   |   |   +---Exe
|                   |   |   +---List
|                   |   |   \---Obj
|                   |   +---include
|                   |   +---settings
|                   |   \---source
|                   \---stm32f103c8t6_keil
|                       +---0_common
|                       |   \---include
|                       +---DebugConfig
|                       +---driver
|                       |   \---include
|                       +---example
|                       |   +---DebugConfig
|                       |   +---Objects
|                       |   \---RTE
|                       |       +---Device
|                       |       |   \---STM32F103C8
|                       |       \---_Target_1
|                       +---Listings
|                       +---Objects
|                       \---RTE
|                           \---Device
|                               \---STM32F103C8
\---Take note quá trình học thanh ghi
    +---Buoi_1
    +---Buoi_10
    |   +---learn some syntax stuff
    |   \---main_test_NVIC
    +---Buoi_2
    +---Buoi_3
    |   +---Blink 1 led
    |   \---Blink n led
    +---Buoi_4
    +---Buoi_5
    |   +---Buoi_5.1
    |   +---Buoi_5.2
    |   +---Buoi_5.3
    |   +---Buoi_5.4
    |   \---Buoi_5.5
    +---Buoi_6
    +---Buoi_7
    |   \---Helper file
    +---Buoi_8
    |   \---Helper file
    \---Buoi_9
        \---Helper_files
            +---driver
            |   \---include
            \---stm32f103c8t6
                \---test_lib
                    +---include
                    \---source
```
