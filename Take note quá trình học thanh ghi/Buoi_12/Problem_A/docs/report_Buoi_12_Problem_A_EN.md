[[TOC]]

## 1. Introduction

**DMA (Direct Memory Access)** is a hardware engine inside the microcontroller that
moves data between memory and peripherals (or memory and memory) **on its own**, without
the CPU executing a single instruction of the copy. The program declares the transfer up
front — source address, destination address, element count, element size — then presses
an enable bit; from that moment the DMA controller arbitrates the bus and performs every
read/write itself. It is best pictured as a **conveyor belt**: set it up once, switch it
on, and the goods move while the operator stands aside.

**Why DMA must exist.** Without it, every byte a peripheral produces or consumes costs
CPU time: either the CPU spins in a **polling** loop asking "ready yet?", or it takes an
**interrupt per data item** — hundreds of context switches just to shovel a buffer. Both
approaches burn cycles, add latency jitter, and scale badly: a 1 Mbps UART stream alone
can saturate a small core. DMA removes the CPU from the data path entirely, so the core
is free to compute (or sleep, saving power) while transfers proceed at bus speed with
deterministic timing.

**Real-world applications today.** DMA is the backbone of virtually every data-moving
subsystem in modern embedded devices:

- **Audio** — I2S/SAI streams feed DACs/codecs from circular DMA buffers (headphones, TWS earbuds, smart speakers).
- **Analog acquisition** — ADCs sample continuously into RAM ring buffers (motor control, battery monitors, biomedical sensors).
- **Communications** — UART/SPI/I2C/Ethernet/USB controllers exchange whole frames without per-byte interrupts.
- **Displays and cameras** — frame buffers are refreshed and captured by dedicated DMA paths (LTDC, DCMI).
- **Storage** — SD-card and flash controllers stream sectors via DMA.

On the STM32F103 used here, the DMA1 controller offers 7 channels shared by these
peripheral classes. This report studies it in its purest form — a memory-to-memory copy —
where the concept is easiest to see and verify.

![The DMA concept on Problem A's datapath: RAM g_src → DMA1 Channel 1 → RAM g_dst at bus speed; the CPU sits idle in while(1) and its usual poll / IRQ involvement is crossed out — it does not interfere with the transfer](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S01_MEM2MEM.png)

## 2. Problem Identification

Through Sessions 1–11 of this project, every peripheral was driven the CPU-centric way:
the core either polled status flags or serviced interrupts, and *all* data passed through
CPU registers. DMA introduces a **third actor** on the bus — and with it a new mental
model (declare-then-delegate) and a new debugging surface (channel registers instead of
program flow). That gap is what this session closes.

The learning problem is deliberately framed at the **register level**, not the library
level: a HAL call that "just works" teaches nothing about *what was written where*. The
project's standing goal — no vendor lock-in, understanding down to the bit — demands
that any library usage be verifiable against the raw registers underneath.

**Problem A (this report):** use DMA1 Channel 1 to copy 8 words (32 bytes) RAM→RAM,
with the CPU parked in an empty `while(1)` — no polling, no interrupts (the "Stage 0"
rule). Small enough to verify exhaustively, complete enough to exercise the whole DMA
programming model (CCR/CNDTR/CPAR/CMAR/ISR).

**Problem A spans three stages** — the same problem re-implemented at three API layers,
all covered by this report:

| Stage | API layer | Status |
|---|---|---|
| 0_1 | HAL (vendor functions + vendor names) | **PASS on board** |
| 0_2 | CMSIS (vendor names only, zero vendor code) | **PASS on board** |
| 0_3 | raw registers (project DSL per `STANDARD.md` — nothing vendor) | **PASS on board** |

Stages 0_2 and 0_3 are **support stages** of 0_1: their purpose is to expose which
components of the vendor API stage 0_1 actually used, and to demonstrate the extraction
technique — read the vendor source, keep only the register writes, re-verify on board.

Problem B — the timer-paced sibling problem of Session 12 — has its own report; the two
share only the session map (section 4.1) and the acceptance method.

## 3. Objective

Acceptance criteria set before writing any code:

- **Build:** `iarbuild` on config `DMA_B12_S01_HAL` completes with **0 errors, 0 warnings**.
- **Function:** after one run on real hardware, `g_dst == g_src` for all 8 words.
- **Proof of completion in registers:** `DMA1_ISR.TCIF1 = 1` and `DMA1_CNDTR1 = 0`.
- **Stage-0 rule respected:** no polling loop, no interrupt enabled (`TCIE/HTIE/TEIE = 0`); the CPU contributes nothing after `EN = 1`.
- **Minimal vendor footprint:** compile only the HAL modules strictly required (target: 1 of ≈30), so later stages can drop HAL without rework.
- **Evidence captured, not recalled:** C-SPY register dumps and Live Watch logs saved to `Problem_A/logs/`, screenshots to `Problem_A/assets/`.
- **Fingerprint recorded for reuse:** the register set {CCR, CNDTR, CPAR, CMAR, ISR} becomes the reference that stages 0_2 and 0_3 must reproduce exactly.

## 4. Methodology and Progress

### 4.1 Method — one problem, three API rungs, one fingerprint

The core method of Session 12 is a **priority ladder**: solve the identical problem at
**HAL** (rung 0_1, simplest), then **CMSIS** (rung 0_2, readable register names), then
**raw registers** (rung 0_3, project DSL). Acceptance is a **diff of register
fingerprints**: if all three implementations leave the same values in
`CCR/CNDTR/CPAR/CMAR`, they are provably the same thing at the hardware level — the
library was only ever a way of *typing*, not a source of behavior.

![The Session 12 API ladder: the application descends through HAL / CMSIS / raw registers to the hardware; the register bank on the left shows exactly which DMA registers this problem touches (green = written, yellow = read); the arrow marks increasing hardware control and precision](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_api_ladder.png)

Problem A's architecture — the one datapath all three stages must reproduce:

```
Problem A architecture (shared by stages 0_1 / 0_2 / 0_3)

   g_src[8] @ 0x20000000                        g_dst[8] @ 0x20000064
  +----------------+     DMA1 Channel 1        +----------------+
  |  RAM  (source) | ========================> |  RAM  (dest)   |
  +----------------+   MEM2MEM=1, 32-bit word  +----------------+
                       MINC=1, PINC=1, N=8
                       one-shot (CIRC=0)

   CPU: while(1) { }   -- no polling, no interrupts (Stage-0 rule)
```

This report covers the whole **Problem A row** of the session map below. The matrix is
now complete — all six cells PASS on board; only the application layer remains.

![The Session 12 map, final state: two problems × three API rungs, 6/6 cells PASS on board with the same register fingerprint per problem; the dashed application layer is the only piece left](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_overall_Roadmap.png)

### 4.2 Toolchain and build profiles

Hardware: STM32F103C8T6 (Blue Pill), flashed and debugged over **ST-Link V2, pure SWD**,
inspected with **C-SPY** (IAR EWARM 8.3). The project keeps **one source list** and
splits it per build profile with `<excluded>` flags; each profile enables only its own
defines. Rung 0_1 gets a dedicated profile **`DMA_B12_S01_HAL`** (`STM32F103xB` +
`USE_HAL_DRIVER`): the compile list is just the stage's main file + `stm32f1xx_hal_dma.c`
+ `startup` + `cortex_m3.c`, every legacy LL / smoke-test file excluded — so the old LL
`Debug` build stays untouched. Notably, `hal.c` / `system_stm32f1xx.c` are **not
needed** (startup does not call `SystemInit`; the linker discards unused HAL code).

The build↔board loop at the right of the figure is the project's hardware boundary:
the AI owns "does it BUILD" (`iarbuild` CLI), the user owns "does it WORK" (on-board
run); hardware truth crosses back only as captured logs.

**Adding / removing a flag (define).** In the IAR GUI: *Project → Options… → C/C++
Compiler → Preprocessor → Defined symbols* — one symbol per line, set **per
configuration** (select the profile in the workspace drop-down first). Adding a line
defines the flag for that profile only; deleting the line removes it. The same list is
stored per configuration as the `<CCDefines>` option inside `test.ewp`, which is why a
profile's flag changes show up as reviewable diffs in git.

![IAR configuration — debug profile setup: one project fans out to five build profiles (only the enabled flags differ; the legend decodes each flag); the hatched profile (Debug_Vendor) is kept in the project but no longer used; all profiles feed iarbuild — PASS flashes to the board, FAIL loops back to fix-and-rebuild](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_IAR_config_debug_profile_Setup.png)

### 4.3 Stage 0_1 — HAL implementation (evidence in code)

Each (problem × stage) cell is one self-contained file living beside the hand library's
own `source/main.c` — `main_b12_A_S01.c` / `_S02.c` / `_S03.c` (and the `main_b12_B_*`
trio for Problem B). The mains are *our* application code; vendor material never enters
the hand tree — stages that need HAL/CMSIS *call into* the warehouse
(`Manufacturer_Package/`) through the project's include paths. The cell to run is
selected by keeping exactly **one** `main_b12_*.c` included in the build (IAR:
include/exclude from build).
*Historical note:* the cells first shared one `#ifdef`-switched file — the "labels are
macro names, test with `#ifdef`, never `#if ==`" lesson in 4.7 dates from that layout.
The complete Stage 0_1 core:

```c
static void Stage01_MemToMem(uint32_t *src, uint32_t *dst)
{
    __HAL_RCC_DMA1_CLK_ENABLE();                        /* DMA1 clock on         */
    hdma.Instance                 = DMA1_Channel1;
    hdma.Init.Direction           = DMA_MEMORY_TO_MEMORY;
    hdma.Init.PeriphInc           = DMA_PINC_ENABLE;    /* source auto-increment */
    hdma.Init.MemInc              = DMA_MINC_ENABLE;    /* dest auto-increment   */
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;/* 32-bit                */
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_NORMAL;         /* one-shot              */
    hdma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma);
    HAL_DMA_Start(&hdma, (uint32_t)src, (uint32_t)dst, 8u);  /* fire            */
    /* NO polling, NO interrupts: read g_dst + CNDTR in C-SPY */
}
```

Two HAL calls and one clock macro — that is the entire vendor surface of stage 0_1.
Everything after `HAL_DMA_Start()` happens in silicon.

### 4.4 Stage 0_2 — CMSIS: extracting the API

The extraction technique: **read the vendor source, keep only what it writes.** Tracing
`stm32f1xx_hal_dma.c` line by line shows the two HAL calls of 4.3 collapse to ~7 register
writes hiding under ~600 lines of vendor code. Stage 0_2 performs those writes directly;
the only vendor material left is *names* — `stm32f103xb.h` is pure vocabulary
(`#define`s, structs, bit masks), zero behavior. Every line is annotated with the HAL
call it replaces:

```c
#include "stm32f103xb.h"     /* CMSIS device header: names only, no code */

static void Stage02_MemToMem(uint32_t *src, uint32_t *dst)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;         /* was __HAL_RCC_DMA1_CLK_ENABLE */

    DMA1_Channel1->CCR &= ~DMA_CCR_EN;        /* HAL_DMA_Init precondition:    */
                                              /* configure only while disabled */
    DMA1_Channel1->CCR = DMA_CCR_MEM2MEM      /* was Init.Direction = MEM2MEM  */
                       | DMA_CCR_MSIZE_1      /* was Init.MemDataAlignment=WORD*/
                       | DMA_CCR_PSIZE_1      /* was Init.PeriphDataAlignment  */
                       | DMA_CCR_MINC         /* was Init.MemInc = ENABLE      */
                       | DMA_CCR_PINC;        /* was Init.PeriphInc = ENABLE   */
    DMA1_Channel1->CNDTR = 8u;                /* was HAL_DMA_Start count arg   */
    DMA1_Channel1->CPAR  = (uint32_t)src;     /* source      (DIR=0: CPAR=src) */
    DMA1_Channel1->CMAR  = (uint32_t)dst;     /* destination                   */
    DMA1_Channel1->CCR  |= DMA_CCR_EN;        /* fires now -> CCR = 0x00004AC1 */
}
```

A useful discovery fell out of the build system here: because the stage label lives in
the *source*, building the old `DMA_B12_S01_HAL` profile compiles this same 0_2 code —
the HAL modules are still compiled but nothing calls them, so the linker strips them and
**both profiles produce the same binary**. The library's presence or absence changes
nothing once the behavior is hand-written.

### 4.5 Stage 0_3 — raw DSL: owning the names

The final stage removes the vendor's *vocabulary* too. The register map is hand-written
from RM0008 chapter 13 into `dma.h` — the project's first DMA map — using the
`BUNION`/`RSTRUCT` DSL of `STANDARD.md`: seven identical channels become one 20-byte
`DMA_CHANNEL_TypeDef` stacked in an array (`CH[0]` = the manual's channel 1), read-only
flags are `const`, and a bit-band twin (`DMA_BITBAND_TypeDef`) mirrors every bit. The
placed instance `DMA1 @ 0x40020000` joins `stm32f103c8t6.h`. The map's layout was
verified with host-side `_Static_assert`s (channel stride = 20 bytes, `CH[4].CCR` at
offset 0x58) before touching the board:

```c
#include <stm32f103c8t6.h>   /* hand-written map: RM0008 only, zero vendor */

static void Stage03_MemToMem(uint32_t *src, uint32_t *dst)
{
    RCC.AHB_ENR.BITS.DMA1 = 1;          /* clock first, always               */

    DMA1.CH[0].CCR.REG = 0;             /* known state; config only while EN=0 */
    DMA1.CH[0].CCR.BITS.MEM2MEM = 1;    /* RAM->RAM: no peripheral request   */
    DMA1.CH[0].CCR.BITS.MSIZE   = 2;    /* 2 = 32-bit memory cells           */
    DMA1.CH[0].CCR.BITS.PSIZE   = 2;    /* 2 = 32-bit "peripheral" cells     */
    DMA1.CH[0].CCR.BITS.MINC    = 1;    /* walk the dest array               */
    DMA1.CH[0].CCR.BITS.PINC    = 1;    /* walk the source array             */
    DMA1.CH[0].CNDTR.BITS.NDT   = 8u;   /* 8 beats                           */
    DMA1.CH[0].CPAR = (uint32_t)src;    /* source (DIR=0: CPAR is "from")    */
    DMA1.CH[0].CMAR = (uint32_t)dst;    /* destination                       */
    DMA1.CH[0].CCR.BITS.EN = 1;         /* fires now -> CCR = 0x00004AC1     */
}
```

Same ~7 writes as 0_2, but every name is now readable *and owned*: `CCR.BITS.MEM2MEM`
instead of a mask, field-per-line writes that document themselves. Stages 0_2/0_3 build
on a dedicated vendor-free profile, **`DMA_B12_S02_03_BareMetal`** (define `STM32F103xB`
only; compile list: the stage's `main_b12_*.c` + startup + `cortex_m3.c`).

### 4.6 Verification method

1. Flash via ST-Link, run through the stage's `StageXX_MemToMem()`, halt.
2. **Register fingerprint:** read DMA1 Channel 1 in the C-SPY Registers window with every
   bit-field expanded; dump to `Problem_A/logs/DMA1.log`.
3. **Memory check:** compare `g_src[]` / `g_dst[]` in Live Watch (decimal and hex);
   logs `LiveWatch-binary.log` / `LiveWatch-hexa.log`.
4. Repeat per stage and **diff the fingerprints**: 0_1 recorded the reference; 0_2 and
   0_3 must reproduce it bit-for-bit (they did — `CCR1 = 0x00004AC1`, `CNDTR = 0`,
   `TCIF1 = 1`, `g_dst == g_src` at all three stages).

### 4.7 Progress

| Step | Outcome |
|---|---|
| Write the Stage 0_1 main (now `main_b12_A_S01.c`) | done |
| Wire `test.ewp` profile `DMA_B12_S01_HAL` | done — old configs untouched |
| Build #1 | FAIL — 3× `Pe014` from `#if STAGE == 0_1` (integer-only `#if`) |
| Fix: `#ifdef STAGE_0_1` labels | done — recorded in `bug_log` |
| Build #2 | 0 errors, 0 warnings |
| On-board run + capture (0_1) | **PASS** — evidence in `Problem_A/logs/` and `assets/` |
| Stage 0_2: trace HAL → CMSIS writes; wire `DMA_B12_S02_03_BareMetal` | done — build 0 err / 0 warn |
| On-board run (0_2) | **PASS** — same fingerprint as 0_1 |
| Stage 0_3: write `dma.h` map (BUNION/RSTRUCT) + host-side layout asserts | done — build 0 err / 0 warn |
| On-board run (0_3) | **PASS** — same fingerprint; Problem A row complete |

## 5. Result and Evaluation

### 5.1 Register fingerprint — DMA1 · Channel 1

In every screenshot below, **red boxes mark live/changing values** (the proof the
transfer ran) and **blue boxes mark configuration/fingerprint values** (the proof it was
set up as coded).

*Table 1 — channel-1 registers after the run:*

| Register | Value | Meaning |
|---|---|---|
| DMA1_ISR | 0x00000007 | GIF1+TCIF1+HTIF1 → **TCIF1 = 1** (transfer complete) |
| DMA1_CCR1 | 0x00004AC1 | channel configuration (decoded in Table 2) |
| DMA1_CNDTR1 | 0x00000000 | 0 words remaining → all 8 transferred |
| DMA1_CPAR1 | 0x20000000 | source = `&g_src[0]` |
| DMA1_CMAR1 | 0x20000064 | destination = `&g_dst[0]` |

*Table 2 — decoding `CCR1 = 0x00004AC1` (bit-fields read directly from C-SPY):*

| Bit-field | Value | Meaning |
|---|---|---|
| MEM2MEM | 1 | memory→memory copy |
| MSIZE / PSIZE | 0b10 / 0b10 | 32-bit (word) elements on both sides |
| MINC / PINC | 1 / 1 | both addresses auto-increment |
| CIRC | 0 | one-shot |
| TCIE / HTIE / TEIE | 0 | all interrupts off (per the Stage 0 rule) |
| EN | 1 | channel enabled |

→ a 100% match with `hdma.Init` in the code of section 4.3.

![C-SPY Registers window — DMA1 channel 1 with every bit-field expanded; red boxes = live values (ISR with TCIF1=1, CNDTR1=0), blue boxes = fingerprint values (CCR1=0x4AC1 with MEM2MEM=1 and EN=1, CPAR1=0x20000000, CMAR1=0x20000064) — matching Tables 1 and 2 exactly](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S01_DMA1_full_reg.png)

### 5.2 Memory verification

*Table 3 — does `g_dst` equal `g_src` (Live Watch, hex):*

| idx | g_src | g_dst | idx | g_src | g_dst |
|---|---|---|---|---|---|
| [0] | 0x11111111 | 0x11111111 | [4] | 0x55555555 | 0x55555555 |
| [1] | 0x22222222 | 0x22222222 | [5] | 0x66666666 | 0x66666666 |
| [2] | 0x33333333 | 0x33333333 | [6] | 0x77777777 | 0x77777777 |
| [3] | 0x44444444 | 0x44444444 | [7] | 0x88888888 | 0x88888888 |

![Before the run: g_dst[] all zeros (red box — about to change), g_src[] holds the pattern (blue box); the Location column (blue) already shows the fixed addresses 0x20000064 / 0x20000000](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S01_staged.png)

![After the run: g_dst[] equals g_src[] — the DMA copy succeeded (red boxes on both value columns; IAR Live Watch displays decimal by default: 286331153 = 0x11111111)](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S01_latter(bin).png)

![Same data with the display switched to hex (right-click → Format → Hexadecimal): g_dst[] == g_src[] = 0x11111111 … 0x88888888 — the pattern reads off directly instead of via decimal arithmetic](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S01_latter(hex).png)

### 5.3 Evaluation against the objectives

| Objective (section 3) | Verdict |
|---|---|
| Build 0 errors / 0 warnings | **met** (build #2) |
| `g_dst == g_src` after one run | **met** (Table 3) |
| `TCIF1 = 1`, `CNDTR = 0` | **met** (Table 1) |
| No polling, no interrupts | **met** (`TCIE/HTIE/TEIE = 0`, empty `while(1)`) |
| Minimal vendor footprint | **met** — 1/≈30 modules, 2/12 functions (section 5.4) |
| Evidence captured to logs/assets | **met** (`Problem_A/logs/`, `Problem_A/assets/`) |
| Fingerprint recorded for 0_2 / 0_3 | **met** (Tables 1–2 are the reference) |

### 5.4 Vendor-API footprint — stage by stage

The vendor supplies two separable things: **functions** (behavior — code that runs) and
**names** (vocabulary — `#define`s and structs). Each stage strips one of them.

**Stage 0_1 — HAL: functions + names.** HAL is used at a deliberate *minimum* — an
ignition switch to get on the air quickly. At module level, 1 of ≈30 HAL driver files is
compiled (~3%); at function level, 2 of the 12 public `HAL_DMA_*` functions are called
(~17%). Still, ~600 vendor lines run for what is really ~7 register writes.

![Vendor API usage in Problem A at stage 0_1, with the two counting units defined by example: 1 module = 1 vendor driver file (only stm32f1xx_hal_dma.c of 30 is compiled, ~3%); 1 function = 1 public API inside that file (only HAL_DMA_Init + HAL_DMA_Start of 12 are called, ~17%); green = used, gray = stripped by the linker](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S01_api_usage.png)

**Stage 0_2 — CMSIS: names only.** Vendor functions drop to **zero** — the ~7 writes are
hand-written. Ten vendor names remain, all from the device header `stm32f103xb.h`
(`DMA1_Channel1`, `RCC_AHBENR_DMA1EN`, the six `DMA_CCR_*` masks, `DMA_Channel_TypeDef`,
`RCC`) — vocabulary with zero behavior behind it.

**Stage 0_3 — raw DSL: nothing vendor.** The names come from the hand-written `dma.h`
(RM0008 chapter 13), the behavior from the same 7 writes. Vendor footprint: **0 modules,
0 functions, 0 names** — while the board fingerprint stays bit-identical
(`CCR1 = 0x00004AC1` at all three stages).

![Vendor-API footprint of Problem A across the three stages: the vendor's FUNCTIONS row is stripped at 0_2 (traced into 7 hand-written writes) and its NAMES row at 0_3 (own dma.h map); orange = vendor material still in the build, green check = replaced by a hand-written equivalent — the fingerprint stays 0x4AC1 throughout](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/Problem_A/assets/fig_12_A_S123_api_footprint.png)

### 5.5 Lessons learned

- **mem2mem is too fast to watch `CNDTR` count down.** The 8 words rip through at bus
  speed (<1 µs) — every halt already shows `CNDTR = 0`. The proof it ran is
  `CNDTR=0` + `TCIF1=1` + a full `g_dst`. A timer-paced transfer (Problem B) is where
  the countdown becomes visible.
- **IAR Live Watch defaults to decimal** (`286331153` = `0x11111111`); switch to hex via
  right-click → Format → Hexadecimal.
- **The Registers window must be refreshed** (click a register) before it shows real values.
- **The preprocessor's `#if` compares integers only** — labels like `0_1` must be selected
  with `#ifdef`, never `#if ==` (fixed; recorded in `bug_log`).
- **An IAR "configuration" lives in both `.ewp` and `.ewd`;** creating a config is GUI
  work (it touches device/linker settings); the shared source list is split per config
  with `<excluded>` flags.
- **The build profile does not choose the stage — the source labels do.** With
  `STAGE_0_2` active, even the HAL profile builds the CMSIS code (its HAL modules
  compile but are dead-stripped: nothing calls them). Both profiles then produce the
  same binary — an accidental but perfect control experiment.
- **Compile-time checks beat board time.** The `dma.h` layout (20-byte channel stride,
  `CH[4].CCR` at 0x58) was proven with host-side `_Static_assert`s before flashing —
  a wrong map would have *built* fine and failed only on hardware.

## 6. References

1. STMicroelectronics, *RM0008 — STM32F101xx/102xx/103xx/105xx/107xx Reference Manual*, ch. 13 "DMA controller" (register definitions: CCR, CNDTR, CPAR, CMAR, ISR/IFCR).
2. STMicroelectronics, *STM32F103x8/B Datasheet* — local copy: `docs/datasheets/stm32f10xxx.pdf`.
3. STMicroelectronics, *UM1850 — Description of STM32F1 HAL and low-layer drivers* (HAL_DMA API).
4. Vendor driver source: `stm32f1xx_hal_dma.c` (STM32CubeF1 package, mirrored in this repository) — ground truth for the 12-function count of section 5.4.
5. Project coding standard: `STANDARD.md` (register-map DSL used at rung 0_3).
6. Captured on-board evidence: `Buoi_12/Problem_A/logs/` (`DMA1.log`, `LiveWatch-hexa.log`, `LiveWatch-binary.log`, `runtime_buoi12.log`).

[[LOF]]
