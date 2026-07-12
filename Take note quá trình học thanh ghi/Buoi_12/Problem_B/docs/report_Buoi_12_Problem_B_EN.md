[[TOC]]

## 1. Introduction

Problem A showed DMA as a **conveyor belt**: declare a transfer, press enable, and the
hardware copies with the CPU standing aside. But a belt that dumps everything at bus
speed is only half the story — most real DMA work is **paced by events**. A peripheral
raises a *DMA request* each time it is ready, and the DMA controller moves exactly one
element per request. That is the belt with a **metronome**.

**Why event-paced DMA matters.** It is the mechanism behind virtually every periodic
I/O pattern in embedded systems: a timer beats, and on every beat one sample moves —
no interrupt handler, no CPU wake-up, perfect timing jitter-free at the hardware level:

- **Waveform generation** — a timer paces DAC (or GPIO) writes from a RAM pattern: audio tones, arbitrary signal generators, WS2812 LED-strip bitstreams.
- **PWM pattern engines** — timer update events stream new duty-cycle values into CCR registers (motor ramps, LED dimming curves).
- **Display scanning** — LED-matrix and 7-segment multiplexing at a fixed refresh rate with zero CPU load.
- **Periodic sampling** — the mirror image: a timer paces ADC conversions into a ring buffer.

**Problem B (this report):** the smallest possible instance of that pattern on the
STM32F103 — **blink the on-board LED (PC13) with the CPU fully halted**. TIM1 overflows
every 400 ms and raises a DMA request (not an interrupt); DMA1 Channel 5 answers by
writing the next word of a 2-word pattern into `GPIOC_ODR`; **circular mode** reloads
the counter forever. The LED blinks even while the debugger holds the core at a
breakpoint — the strongest possible proof that the CPU is not involved.

![Problem B datapath: TIM1 update event → DMA1 Channel 5 (circular) → GPIOC_ODR → LED PC13; the CPU does not interfere — the transfer is paced by the timer, not by code](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_B/assets/fig_12_S01_DMA_datapath.png)

## 2. Problem Identification

Problem A left two questions deliberately open:

1. **Nobody saw the belt move.** mem2mem rips through at bus speed (<1 µs), so every
   halt already showed `CNDTR = 0`. To *watch* DMA work — the counter stepping down,
   the destination changing — the transfer must be slowed to human speed by an
   external pacer.
2. **The peripheral-request side was untouched.** Problem A used `MEM2MEM`, which needs
   no request line. Real DMA is driven by peripheral requests routed through a **fixed
   channel mapping** (RM0008 Table 78: TIM1_UP is hard-wired to DMA1 Channel 5) and
   gated by request-enable bits (`TIM1_DIER.UDE`) — a whole mechanism absent from
   Problem A.

Problem B closes both gaps, and adds two new register-level concepts on the way:
**circular mode** (`CIRC = 1`: `CNDTR` self-reloads, the transfer never "completes")
and **memory→peripheral direction** with a fixed destination (`PINC = 0`).

**Problem B spans three stages** — the same problem re-implemented at three API layers,
all covered by this report:

| Stage | API layer | Status |
|---|---|---|
| 0_1 | HAL (vendor functions + vendor names) | **PASS on board** |
| 0_2 | CMSIS (vendor names only, zero vendor code) | **PASS on board** |
| 0_3 | raw registers (project DSL per `STANDARD.md` — nothing vendor) | **PASS on board** |

Stages 0_2 and 0_3 are **support stages** of 0_1: their purpose is to expose which
components of the vendor API stage 0_1 actually used, and to demonstrate the extraction
technique — read the vendor source, keep only the register writes, re-verify on board.

Problem A — the mem-to-mem sibling problem of Session 12 — has its own report; the two
share only the session map (section 4.1) and the acceptance method.

## 3. Objective

Acceptance criteria set before writing any code:

- **Build:** `iarbuild` on config `DMA_B12_S01_HAL` (with `PROBLEM_B` selected) completes with **0 errors, 0 warnings**.
- **Function:** LED PC13 blinks with a 400 ms half-period — and **keeps blinking while the core is halted** at a breakpoint (the "CPU not involved" proof).
- **Visible countdown:** `DMA1_CH5.CNDTR` can be watched stepping `2 → 1 → 2` in Live Watch — the observation Problem A could not offer.
- **Correct pacing chain in registers:** `TIM1_DIER.UDE = 1` (DMA request, *not* `UIE` interrupt), `TIM1_CR1.CEN = 1`, `PSC = 7999`, `ARR = 399`.
- **Stage-0 rule respected:** no polling loop, no interrupt enabled; the CPU contributes nothing after start-up.
- **Minimal vendor footprint:** compile only the HAL modules strictly required (target: 3 of ≈30 — DMA, TIM, GPIO).
- **Evidence captured, not recalled:** annotated C-SPY screenshots in `Problem_B/assets/`; the register set {CCR, CNDTR, CPAR, CMAR} + {CR1, DIER, PSC, ARR} becomes the fingerprint that stages 0_2 and 0_3 must reproduce.

## 4. Methodology and Progress

### 4.1 Method — one problem, three API rungs, one fingerprint

Problem B walks the same **priority ladder** as Problem A: solve at **HAL** (rung 0_1),
re-solve at **CMSIS** (rung 0_2), re-solve at **raw registers** (rung 0_3), then **diff
the register fingerprints** — if all three leave the same values in the DMA channel and
TIM1 registers, they are provably the same thing at the hardware level.

![The Session 12 API ladder: the application descends through HAL / CMSIS / raw registers to the hardware; the register bank on the left shows which DMA registers the session touches (green = written, yellow = read); the arrow marks increasing hardware control and precision](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_api_ladder.png)

Problem B's architecture — the one datapath all three stages must reproduce:

```
Problem B architecture (shared by stages 0_1 / 0_2 / 0_3)

   g_led_pattern[2] @ 0x20000000            GPIOC_ODR @ 0x4001100C
  +------------------------+   DMA1 Channel 5   +---------------+     PC13
  | RAM: { 0x2000, 0x0 }   | =================> | GPIO port C   | --> LED
  +------------------------+  MEM->PERIPH, word +---------------+  (active-low)
              ^               MINC=1, PINC=0
              |               CIRCULAR, N=2
        one word per beat
              |
  +------------------------+
  |  TIM1 update event     |   PSC=7999, ARR=399
  |  DIER.UDE=1 (request,  |   8 MHz HSI / 8000 / 400
  |  NOT an interrupt)     |   -> one beat every 400 ms
  +------------------------+

   CPU: while(1) { }   -- no polling, no interrupts (Stage-0 rule)
```

This report covers the whole **Problem B row** of the session map below. The matrix is
now complete — all six cells PASS on board; only the application layer remains.

![The Session 12 map, final state: two problems × three API rungs, 6/6 cells PASS on board with the same register fingerprint per problem; the dashed application layer is the only piece left](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_overall_Roadmap.png)

### 4.2 Toolchain and build profiles

Hardware and toolchain are identical to Problem A: STM32F103C8T6 (Blue Pill), ST-Link V2
over pure SWD, C-SPY in IAR EWARM 8.3. Problem B reuses the **same build profile
`DMA_B12_S01_HAL`** (`STM32F103xB` + `USE_HAL_DRIVER`). Each (problem × stage) cell is
one self-contained file living beside the hand library's own `source/main.c` — this
problem's trio is `main_b12_B_S01.c` / `_S02.c` / `_S03.c`. The mains are *our*
application code; stages that need HAL/CMSIS *call into* the vendor warehouse
(`Manufacturer_Package/`) through include paths. The cell to run is selected by keeping
exactly **one** `main_b12_*.c` included in the build. Two more
vendor modules join the compile list for this problem: `stm32f1xx_hal_gpio.c` (pin
setup) and `stm32f1xx_hal_tim.c` (timer base), alongside the `stm32f1xx_hal_dma.c`
already wired for Problem A.

**Adding / removing a flag (define).** In the IAR GUI: *Project → Options… → C/C++
Compiler → Preprocessor → Defined symbols* — one symbol per line, set **per
configuration** (select the profile in the workspace drop-down first). Adding a line
defines the flag for that profile only; deleting the line removes it. The same list is
stored per configuration as the `<CCDefines>` option inside `test.ewp`, which is why a
profile's flag changes show up as reviewable diffs in git.

![IAR configuration — debug profile setup: one project fans out to five build profiles (only the enabled flags differ; the legend decodes each flag); the hatched profile (Debug_Vendor) is kept in the project but no longer used; all profiles feed iarbuild — PASS flashes to the board, FAIL loops back to fix-and-rebuild](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_IAR_config_debug_profile_Setup.png)

### 4.3 Stage 0_1 — HAL implementation (evidence in code)

The pattern is two whole-word `ODR` images, replayed forever by circular DMA —
`[0] = bit 13 set` (PC13 high, LED **off**: the Blue Pill LED is active-low),
`[1] = all clear` (PC13 low, LED **on**):

```c
static uint32_t g_led_pattern[2] = { (1uL << 13), 0uL };
```

The complete Stage 0_1 core — timer as metronome, DMA as belt, one hook between them:

```c
static void Stage01_LedByTimDma(void)
{
    /* -- timer: 8 MHz HSI / 8000 = 1 kHz tick, reload every 400 ticks -- */
    __HAL_RCC_TIM1_CLK_ENABLE();
    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = 8000u - 1u;  /* 1 tick = 1 ms            */
    htim1.Init.Period            = 400u - 1u;   /* update event each 400 ms */
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.RepetitionCounter = 0u;          /* TIM1: fire every reload  */
    HAL_TIM_Base_Init(&htim1);                  /* writes PSC/ARR/CR1       */

    /* -- DMA: memory (pattern) -> peripheral (ODR), circular, word cells -- */
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma.Instance                 = DMA1_Channel5;   /* TIM1_UP lives here  */
    hdma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc           = DMA_PINC_DISABLE;   /* ODR never moves  */
    hdma.Init.MemInc              = DMA_MINC_ENABLE;    /* walk the pattern */
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_CIRCULAR;    /* never "done": no TC  */
    HAL_DMA_Init(&hdma);
    HAL_DMA_Start(&hdma, (uint32_t)g_led_pattern, (uint32_t)&GPIOC->ODR, 2u);

    /* -- hook them together and press play -- */
    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);   /* DIER.UDE = 1         */
    HAL_TIM_Base_Start(&htim1);                     /* CR1.CEN  = 1 - GO    */
    /* NO poll, NO interrupt: from here the LED blinks with the CPU halted. */
}
```

One debugging trick worth recording — **needed at stages 0_1/0_2 only**:
`DMA1_Channel5` and `TIM1` are preprocessor macros, invisible to C-SPY Live Watch. Two
`__root` pointer constants make the whole register structs watchable (`*g_dbg_dma5`,
`*g_dbg_tim1`) without being dead-stripped. Stage 0_3 deletes both pointers — see 4.5
for why they become unnecessary:

```c
__root DMA_Channel_TypeDef * const g_dbg_dma5 = DMA1_Channel5;
__root TIM_TypeDef         * const g_dbg_tim1 = TIM1;
```

### 4.4 Stage 0_2 — CMSIS: extracting the API

The extraction technique: **read the vendor source, keep only what it writes.** Tracing
`stm32f1xx_hal_gpio.c` / `_tim.c` / `_dma.c` shows the five HAL calls and two macros of
4.3 collapse to ~15 register writes hiding under ~900 lines of vendor code. Two of them
are worth calling out: the GPIO leg is just *one* `CRH` nibble write after the clock
enable, and `HAL_TIM_Base_Init` secretly ends with `EGR = UG` — `PSC` is a *preload*
register, and without that forced update event the first period would run at the old
prescaler. Stage 0_2 reproduces both faithfully:

```c
#include "stm32f103xb.h"     /* CMSIS device header: names only, no code */

static void Stage02_Led_Gpio_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;       /* was __HAL_RCC_GPIOC_CLK_ENABLE */
    GPIOC->CRH = (GPIOC->CRH
               & ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13))  /* wipe pin-13 nibble   */
               |   GPIO_CRH_MODE13_1;         /* was HAL_GPIO_Init: MODE13=10  */
}                                             /* (2 MHz), CNF13=00 = OUTPUT_PP */

static void Stage02_LedByTimDma(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;       /* was __HAL_RCC_TIM1_CLK_ENABLE */
    TIM1->ARR = 400u - 1u;                    /* was Init.Period               */
    TIM1->PSC = 8000u - 1u;                   /* was Init.Prescaler            */
    TIM1->RCR = 0u;                           /* was Init.RepetitionCounter    */
    TIM1->EGR = TIM_EGR_UG;                   /* HAL tail: latch PSC NOW       */

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;         /* was __HAL_RCC_DMA1_CLK_ENABLE */
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;        /* config only while disabled    */
    DMA1_Channel5->CCR = DMA_CCR_DIR | DMA_CCR_CIRC | DMA_CCR_MINC
                       | DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1;  /* was HAL_DMA_Init */
    DMA1_Channel5->CNDTR = 2u;                /* was HAL_DMA_Start count arg   */
    DMA1_Channel5->CPAR  = (uint32_t)&GPIOC->ODR;  /* dest  (DIR=1: CPAR=dst)  */
    DMA1_Channel5->CMAR  = (uint32_t)g_led_pattern;/* source                   */
    DMA1_Channel5->CCR  |= DMA_CCR_EN;        /* armed -> CCR = 0x00000AB1     */

    TIM1->DIER |= TIM_DIER_UDE;               /* was __HAL_TIM_ENABLE_DMA      */
    TIM1->CR1  |= TIM_CR1_CEN;                /* was HAL_TIM_Base_Start - GO   */
}
```

### 4.5 Stage 0_3 — raw DSL: owning the names, and simpler debugging

The final stage removes the vendor's *vocabulary* too. Problem B pulls **four**
hand-written maps: the new `dma.h` (written for Problem A's 0_3, reused here — its
channel array makes `CH[4]` = the manual's channel 5) plus the library's existing
`advanced_timer.h`, `gpio.h`, `rcc.h`. All are placed instances in `stm32f103c8t6.h`:

```c
#include <stm32f103c8t6.h>   /* hand-written map: RM0008 only, zero vendor */

static void Stage03_Led_Gpio_Init(void)
{
    RCC.APB2_ENR.BITS.IOPC = 1;         /* clock first, always               */
    GPIOC.CRH.BITS.MODE_13 = 2;         /* 2 = output, 2 MHz max             */
    GPIOC.CRH.BITS.CNF_13  = 0;         /* 0 = general-purpose push-pull     */
}

static void Stage03_LedByTimDma(void)
{
    RCC.APB2_ENR.BITS.TIM1 = 1;
    TIM1.ARR = 400u - 1u;               /* update event each 400 ms          */
    TIM1.PSC = 8000u - 1u;              /* 1 tick = 1 ms                     */
    TIM1.RCR = 0u;
    TIM1.EGR.BITS.UG = 1;               /* latch PSC now (0_2 lesson)        */

    RCC.AHB_ENR.BITS.DMA1 = 1;
    DMA1.CH[4].CCR.REG = 0;             /* CH[4] = RM0008 channel 5          */
    DMA1.CH[4].CCR.BITS.DIR   = 1;      /* read memory, write peripheral     */
    DMA1.CH[4].CCR.BITS.CIRC  = 1;      /* CNDTR auto-reloads forever        */
    DMA1.CH[4].CCR.BITS.MINC  = 1;
    DMA1.CH[4].CCR.BITS.MSIZE = 2;      /* 32-bit cells (PINC stays 0)       */
    DMA1.CH[4].CCR.BITS.PSIZE = 2;
    DMA1.CH[4].CNDTR.BITS.NDT = 2u;
    DMA1.CH[4].CPAR = (uint32_t)&GPIOC.ODR;
    DMA1.CH[4].CMAR = (uint32_t)g_led_pattern;
    DMA1.CH[4].CCR.BITS.EN = 1;         /* armed -> CCR = 0x00000AB1         */

    TIM1.DIER.BITS.UDE = 1;             /* DMA request on update - NOT IRQ   */
    TIM1.CR1.BITS.CEN  = 1;             /* GO                                */
}
```

**Checking registers in Live Watch gets *simpler* at this stage — the two `__root`
debug pointers of 4.3 are deleted.** The vendor's `TIM1` was a cast macro
(`#define TIM1 ((TIM_TypeDef *)0x40012C00)`) — no symbol exists, so C-SPY needed the
`g_dbg_*` pointer detour. The hand map instead *places real variables*
(`__root __no_init volatile ADVANCED_TIMER_TypeDef TIM1 @ 0x40012C00;` in
`stm32f103c8t6.h`), so the debugger knows them by name. The whole check procedure is:
type **`DMA1`**, **`TIM1`**, **`GPIOC`** directly into Live Watch, expand, and read
fields by name — `DMA1.CH[4].CNDTR.BITS.NDT` bouncing 2 ↔ 1, `TIM1.DIER.BITS.UDE = 1`,
`GPIOC.ODR.REG` flipping `0x2000 ↔ 0x0000` — no pointer dereference, no per-register
watch entries.

### 4.6 Verification method

1. Flash via ST-Link, run through start-up, then **halt the core** — the LED must keep
   blinking (hardware-only proof).
2. **Watch the belt move:** in Live Watch — via `*g_dbg_dma5` / `*g_dbg_tim1` at stages
   0_1/0_2, or directly by name (`DMA1`, `TIM1`) at stage 0_3 — `CNDTR` steps
   `2 → 1 → 2 → …` one step per 400 ms beat while `CNT` ticks.
3. **Register fingerprint:** capture DMA1_CH5 {CCR, CNDTR, CPAR, CMAR} and TIM1
   {CR1, DIER, CNT, PSC, ARR}; screenshot with annotations (red = live values,
   blue = configuration values).
4. Repeat per stage and **diff the fingerprints**: 0_1 recorded the reference; 0_2 and
   0_3 must reproduce it bit-for-bit (they did — `CCR5 = 0x0AB1`, `DIER = 0x100`,
   `PSC = 7999`, `ARR = 399`, LED blinking with the core halted, at all three stages).

### 4.7 Progress

| Step | Outcome |
|---|---|
| Write the Stage 0_1 main (now `main_b12_B_S01.c`) | done |
| Register `hal_gpio.c` + `hal_tim.c` in `test.ewp` | done — other configs untouched |
| Build (`DMA_B12_S01_HAL`) | 0 errors, 0 warnings |
| On-board run: LED blinks with core halted | **PASS** |
| CNDTR countdown observed in Live Watch | **PASS** — screenshots in `Problem_B/assets/` |
| Stage 0_2: trace HAL → CMSIS writes (GPIO + TIM + DMA) | done — build 0 err / 0 warn |
| On-board run (0_2) | **PASS** — same fingerprint as 0_1 |
| Stage 0_3: reuse `dma.h` + existing TIM/GPIO/RCC maps; drop debug pointers | done — build 0 err / 0 warn |
| On-board run (0_3) | **PASS** — same fingerprint; Problem B row complete |

## 5. Result and Evaluation

### 5.1 Register fingerprint — DMA1 · Channel 5 + TIM1

In both screenshots below, **red boxes mark live/changing values** (`CNDTR`, `CNT` —
the proof the metronome is beating) and **blue boxes mark configuration/fingerprint
values** (the proof it was set up as coded).

*Table 1 — the fingerprint after start-up (read in C-SPY via `*g_dbg_dma5` / `*g_dbg_tim1`):*

| Register | Value | Meaning |
|---|---|---|
| DMA1_CCR5 | 0x00000AB1 | channel configuration (decoded in Table 2) |
| DMA1_CNDTR5 | 2 ↔ 1 (live) | words remaining — reloads to 2 (circular) |
| DMA1_CPAR5 | 0x4001100C | destination = `&GPIOC->ODR` (fixed, PINC=0) |
| DMA1_CMAR5 | 0x20000000 | source = `g_led_pattern[0]` |
| TIM1_CR1 | 0x00000001 | **CEN = 1** — counter running |
| TIM1_DIER | 0x00000100 | **UDE = 1** — update-DMA request; **UIE = 0** (no interrupt) |
| TIM1_CNT | 300 / 341 (live) | counting 0…399, one lap = 400 ms |
| TIM1_PSC | 7999 | 8 MHz / 8000 = 1 kHz tick |
| TIM1_ARR | 399 | update event every 400 ticks = 400 ms |

*Table 2 — decoding `CCR5 = 0x00000AB1` (contrast with Problem A's `0x00004AC1`):*

| Bit-field | Value | Meaning |
|---|---|---|
| MEM2MEM | 0 | not memory→memory — a real peripheral request drives it |
| DIR | 1 | read from memory, write to peripheral |
| **CIRC** | **1** | circular — `CNDTR` self-reloads, transfer never "completes" |
| MSIZE / PSIZE | 0b10 / 0b10 | 32-bit (word) elements on both sides |
| MINC / PINC | 1 / 0 | pattern pointer walks; `ODR` address stays fixed |
| TCIE / HTIE / TEIE | 0 | all interrupts off (per the Stage 0 rule) |
| EN | 1 | channel enabled |

→ a 100% match with `hdma.Init` / `htim1.Init` in the code of section 4.3.

![First capture: DMA1 channel 5 mid-pattern — CNDTR = 1 (red, one word left before reload), CCR = 0x0AB1 / CPAR = 0x4001100C / CMAR = 0x20000000 (blue); below, TIM1 with CR1.CEN = 1, DIER = 0x100 (UDE), CNT = 300 ticking (red), PSC = 7999, ARR = 399 (blue)](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_B/assets/fig_12_B_S01_CNDTR_countdown.png)

![Second capture, moments later: CNDTR has reloaded to 2 (red) — the circular belt restarted by itself; TIM1 CNT = 341 (red) shows the metronome still ticking; every blue configuration value is unchanged](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_B/assets/fig_12_B_S01_CNDTR_reload.png)

### 5.2 The decisive observation

Two captures, taken seconds apart, tell the whole story:

- **`CNDTR` moved `1 → 2`** — it hit zero and *reloaded itself*: circular mode working, one word per timer beat. This is the countdown Problem A was too fast to show.
- **`CNT` moved `300 → 341`** — the metronome ticks on independently.
- **Every configuration register is bit-identical between captures** — nothing but the two live counters changes, because nothing but the hardware is running: the core was halted the entire time, and the LED kept blinking.

![Timing reconstruction of the observation: TIM1 beats every 400 ms (orange = update event, a DMA request via DIER.UDE — no interrupt); CNDTR steps 2 → 1 and self-reloads when it hits 0 (CIRC = 1); ODR bit 13 toggles the active-low LED — all inside the gray band where the core sits halted at a breakpoint, executing nothing](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_B/assets/fig_12_B_S01_halted_cpu_blink.png)

### 5.3 Evaluation against the objectives

| Objective (section 3) | Verdict |
|---|---|
| Build 0 errors / 0 warnings | **met** |
| LED blinks at 400 ms half-period with core halted | **met** (on-board run) |
| `CNDTR` countdown visible, `2 → 1 → 2` | **met** (screenshots, section 5.1) |
| Pacing chain: `UDE=1`, `UIE=0`, `CEN=1`, `PSC=7999`, `ARR=399` | **met** (Table 1) |
| No polling, no interrupts | **met** (`DIER.UIE = 0`, `TCIE/HTIE/TEIE = 0`, empty `while(1)`) |
| Minimal vendor footprint (3/≈30 modules) | **met** — DMA + TIM + GPIO only (section 5.4) |
| Evidence captured; fingerprint recorded for 0_2 / 0_3 | **met** (Tables 1–2 are the reference) |

### 5.4 Vendor-API footprint — stage by stage

The vendor supplies two separable things: **functions** (behavior — code that runs) and
**names** (vocabulary — `#define`s and structs). Each stage strips one of them.

**Stage 0_1 — HAL: functions + names.** Problem B needs three HAL modules instead of
one — `stm32f1xx_hal_dma.c`, `stm32f1xx_hal_tim.c`, `stm32f1xx_hal_gpio.c` (3 of ≈30,
~10%) — and five public functions plus two macros: `HAL_GPIO_Init`, `HAL_TIM_Base_Init`,
`HAL_TIM_Base_Start`, `HAL_DMA_Init`, `HAL_DMA_Start`, `__HAL_TIM_ENABLE_DMA`,
`__HAL_RCC_*_CLK_ENABLE`. Still an ignition switch, not a dependency: ~900 vendor lines
run for what is really ~15 register writes.

![Vendor API usage in Problem B at stage 0_1, with the two counting units defined by example: 1 module = 1 vendor driver file (3 of 30 compiled: hal_dma + hal_tim + hal_gpio, ~10%); 1 function = 1 public API inside those files (5 of 94 called: HAL_GPIO_Init, HAL_TIM_Base_Init/Start, HAL_DMA_Init/Start, ~5%); green = used, gray = stripped by the linker](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_B/assets/fig_12_B_S01_api_usage.png)

**Stage 0_2 — CMSIS: names only.** Vendor functions drop to **zero** — the ~15 writes
are hand-written (section 4.4). About 21 vendor names remain, all from `stm32f103xb.h`:
peripheral macros (`TIM1`, `GPIOC`, `DMA1_Channel5`), bit masks (`TIM_DIER_UDE`,
`TIM_CR1_CEN`, `TIM_EGR_UG`, `GPIO_CRH_MODE13`, the `DMA_CCR_*` family, the
`RCC_*ENR_*` enables) and two struct typedefs — vocabulary with zero behavior behind it.

**Stage 0_3 — raw DSL: nothing vendor.** The names now come from four hand-written maps
(`dma.h` — new this session — plus `advanced_timer.h`, `gpio.h`, `rcc.h`), the behavior
from the same ~15 writes. Vendor footprint: **0 modules, 0 functions, 0 names** — while
the board fingerprint stays bit-identical (`CCR5 = 0x0AB1`, `DIER = 0x100`,
`PSC = 7999`, `ARR = 399`) and debugging gets *simpler* (real placed symbols, no
`__root` helper pointers).

![Vendor-API footprint of Problem B across the three stages: the vendor's FUNCTIONS row is stripped at 0_2 (traced into 15 hand-written writes) and its NAMES row at 0_3 (own maps: dma.h + advanced_timer.h + gpio.h + rcc.h); orange = vendor material still in the build, green check = replaced by a hand-written equivalent — the fingerprint stays 0x0AB1 / 0x100 / 7999 / 399 throughout](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_B/assets/fig_12_B_S123_api_footprint.png)

### 5.5 Lessons learned

- **A DMA request is not an interrupt.** `TIM1_DIER` has two separate enable bits:
  `UIE` (update *interrupt* → CPU runs a handler) and `UDE` (update *DMA request* →
  hardware moves data, CPU untouched). Problem B sets only `UDE` — the LED blinks with
  zero software execution.
- **DMA channels are hard-wired to peripherals.** TIM1_UP can only arrive on DMA1
  Channel 5 (RM0008 Table 78); the channel is chosen by looking it up, not by preference.
- **Circular mode redefines "done".** With `CIRC = 1` there is no transfer-complete
  state to wait for — `CNDTR` reloads forever. The Problem A habit of "check `TCIF`,
  check `CNDTR == 0`" simply does not apply.
- **Word-writes to `ODR` rewrite the whole port.** The 2-word pattern clears every
  other PORTC bit each beat — harmless here (PC13 is the only pin wired), but a real
  application would use `BSRR` set/reset semantics instead.
- **The Blue Pill LED is active-low:** `bit13 = 1` turns it *off*. Getting the pattern
  order wrong costs nothing but confusion — worth one comment in the code.
- **`__root` debug pointers make macro-peripherals watchable — but only vendor stages
  need them.** `DMA1_Channel5` is a cast constant, not a symbol; a `__root` pointer
  variable gives C-SPY a real object to expand, and survives the linker's dead-stripping
  despite being referenced by nothing. At stage 0_3 the hand-written map *places real
  symbols* (`TIM1 @ 0x40012C00`), so both pointers are deleted and Live Watch works by
  name: `DMA1`, `TIM1`, `GPIOC`.
- **The build profile does not choose the stage — the source labels do.** With a 0_2/0_3
  label active, even the HAL profile builds the same hand-written code (its HAL modules
  compile but are dead-stripped). Both profiles then produce the same binary — an
  accidental but perfect control experiment.

## 6. References

1. STMicroelectronics, *RM0008 — STM32F101xx/102xx/103xx/105xx/107xx Reference Manual*: ch. 13 "DMA controller" (Table 78 — DMA1 request mapping: TIM1_UP → Channel 5), ch. 14 "Advanced-control timers (TIM1)" (CR1, DIER.UDE, PSC, ARR).
2. STMicroelectronics, *STM32F103x8/B Datasheet* — local copy: `docs/datasheets/stm32f10xxx.pdf`.
3. STMicroelectronics, *UM1850 — Description of STM32F1 HAL and low-layer drivers* (HAL_DMA, HAL_TIM, HAL_GPIO APIs).
4. Vendor driver sources: `stm32f1xx_hal_dma.c`, `stm32f1xx_hal_tim.c`, `stm32f1xx_hal_gpio.c` (STM32CubeF1 package, mirrored in this repository).
5. Project coding standard: `STANDARD.md` (register-map DSL used at rung 0_3).
6. Captured on-board evidence: `Buoi_12/Problem_B/assets/` (`fig_12_B_S01_CNDTR_countdown.png`, `fig_12_B_S01_CNDTR_reload.png`, annotated).
7. Sibling report: *Session 12 — Problem A: DMA mem-to-mem (HAL, rung 0_1)* — `Buoi_12/Problem_A/docs/report_Buoi_12_Problem_A_EN.docx`.

[[LOF]]
