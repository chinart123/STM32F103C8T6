# STANDARD.md — Bare-Metal C Coding Standard

> Loaded on demand by the code-writer skill (kept out of `AGENTS.md` so the spine
> stays short). Goal: clean, professional bare-metal engineering that *keeps this
> project's proven register-map DSL* — not tutorial mimicry, not HAL/LL.

---

## 1. Register-map model (the backbone — keep it)
- Map every peripheral as a `typedef struct` of registers built with the
  project's `BUNION(NAME, type, field, width, …)` macro (see `0_common/include/define.h`).
  It generates `union { REG; struct { …bitfields… } BITS; }`.
- **Access rule:**
  - Whole-word / masks → `X.REG`
  - Single field, readable intent → `X.BITS.field` *(prefer this for clarity)*
  - Atomic single-bit set/reset → `BSRR`/`BRR`, or the bit-band alias typedef.
- Keep the parallel `RSTRUCT`/`*_BITBAND_TypeDef` for bit-band access
  (`GPIOB_BITBAND.ODR.b12 = 1;`).
- Mark read-only / status bits `const` (e.g. `const HSI_RDY`) so the compiler
  blocks accidental writes. Name gaps `_reserved`, `_reserved1`, …

## 2. Atomicity & concurrency
- Prefer **BSRR/BRR** or **bit-banding** for single-bit changes on shared
  registers — never a non-atomic `REG |=` when an ISR may touch the same reg.
- In ISRs, **clear the status flag** you handled (e.g. `TIM1.SR.REG = 0;` or the
  specific bit) or the interrupt re-fires forever.
- `volatile` on every memory-mapped register object. Don't cache register reads.

## 3. No blocking delays in new code
- No `for(i=0;i<500000;i++);` spin delays (the early Keil `main.c` uses one — do
  not reproduce it). Use the SysTick / non-blocking state-machine pattern from
  the later sessions (`cortex_m3.c` `STK_Init`).

## 4. Comments
- **English, correct spelling.** Fix the legacy `int→long` corruption on any line
  you touch: `longernal→internal`, `longerrupt→interrupt`, `longerface→interface`.
- Keep the RM-aligned column style on register bitfields:
  `//<bit(s)>   <default>   <description>` mirroring ST RM0008.
- Every non-trivial function gets a short doxygen-style header:
  ```c
  /**
   * @brief  Set GPIO pin mode.
   * @param  GPIO  port base (e.g. &GPIOB)
   * @param  PIN   bit-mask of pins (1UL<<12 = pin 12)
   * @param  Mode  a GPIO_MODE enum value
   */
  ```
- Explain *why*, not *what*. The user is learning — a one-line rationale beats a
  paraphrase of the code.

## 5. Naming
- Types: `Xxx_TypeDef` (registers), `XxxInit_TypeDef` (config structs). Enums:
  `PERIPH_THING_VALUE` mirroring the RM bit-pairs
  (`GPIO_MODE_OUTPUT_PUSHPULL_10MHz = (0<<2)|1`).
- Functions: `Periph_Verb` (`GPIO_Mode`, `RCC_Init`). Locals: lower snake / short.
- No magic numbers — use an enum, a named field, or a `1UL<<n` with a comment.

## 6. File & module layout
- `#ifndef _NAME_H_ / #define / #endif` include guards (match existing).
- One peripheral = one `.h` (map + enums + prototypes) + one `.c` (logic).
- Level-0 = raw register structs; Level-1 = an `XxxInit_TypeDef` + `Xxx_Init()`
  driven by **designated initializers** (see `RCCInit_TypeDef` example in `rcc.h`).
  Provide a worked `EXAMPLE` block in the header comment, as `rcc.h` does.

## 7. Clock / flash-latency safety (learned bugs)
- Raising SYSCLK requires flash wait-states first:
  `FSYS>24MHz → LATENCY=1`, `FSYS>48MHz → LATENCY=2`. Set latency *before*
  switching the clock, not after.
- Enable a peripheral's **RCC clock** before touching its registers (a dead
  clock makes every write silently no-op — a classic "compiles, doesn't work").

## 8. What NOT to do
- No HAL, no LL, no CMSIS driver calls. Raw registers only.
- Don't rewrite the `BUNION`/`define.h` metaprogramming unless asked — it's
  load-bearing infrastructure.
- Don't touch `C&C++/` or `Take note …/` files without an explicit request.

---

*When in doubt, read 2–3 real files in `C&C++/Embedded_C99/` and match their
structure exactly, then apply the cleanups above.*
