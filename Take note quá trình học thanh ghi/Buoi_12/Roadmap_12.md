# Buổi 12 — Roadmap DMA (bản viết lại §N): thang API **HAL → CMSIS → raw**

> Bản kế hoạch **trước giờ học**. Kết quả thật nằm ở `Note_12.txt` + snapshot
> source + `runtime_buoi12.log`. Bản `.docx` cùng thư mục là bản trao tay, sinh
> tự động từ chính file này.
>
> **Vì sao viết lại?** Bản cũ (§14) dựng track hãng bằng **LL**. Sau smoke-test
> §16 mới rõ: giữa HAL và thanh ghi raw còn nhiều tầng, và **LL tuy nhanh nhưng
> với người mới là thừa**. Bản này **bỏ hẳn LL**, chỉ đi theo thang ưu tiên bên dưới.

## 0. Thang ưu tiên API (luật xuyên suốt Buổi 12)

| Ưu tiên | Tầng | Vì sao chọn | Lấy code ở đâu |
|---|---|---|---|
| **0** | **HAL** (hãng) | Đơn giản nhất, đúng vai người mới; che hết thanh ghi | `Manufacturer_Package\No.0_C&C++_Industrial_Draft\...` |
| **1** | **CMSIS** (device header) | Tên thanh ghi **dễ đọc** (`DMA1_Channel1->CCR`), vẫn thấy phần cứng | `STM32CubeF1\...\CMSIS\...` |
| **2** | **raw DSL** (tự viết) | **Siêu chính xác**, đúng `STANDARD.md` (`RSTRUCT`/`BUNION`, BSRR/bit-band) | `C&C++\Embedded_C99\Microcontroller\...` |

> **KHÔNG dùng LL** trong buổi này. (Smoke-test §16 còn dùng LL ở MODE 3/4 — đó là
> artefact riêng trong Draft, KHÔNG phải sản phẩm Buổi 12.)

## 1. Mô hình tư duy DMA

- DMA là **băng chuyền**, không phải trợ lý thông minh: nguồn / đích / số lượng /
  cỡ ô khai báo trước (`CPAR` / `CMAR` / `CNDTR` / `CCR`) — nó không tự quyết gì.
- DMA đi chung bus AHB với CPU, có arbiter chia lượt — **rẻ chứ không miễn phí**.
- Câu hỏi trung tâm: *"CPU biết việc xong lúc nào?"* — có 3 cách; **Stage 0 chưa
  dùng cách nào cả** (cấm poll, cấm interrupt — chỉ bật băng chuyền rồi soi C-SPY):

| Cách | Cơ chế | Để dành Stage |
|---|---|---|
| Poll cờ `TCIF` | CPU liếc khay định kỳ | Stage 1 |
| Ngắt `TC` qua NVIC | trợ lý gõ cửa 1 lần/lô | Stage 2 |
| Circular mode | buffer luôn tươi, không "xong" | dùng cho LED nháy vĩnh viễn & về sau |

## 2. Bản đồ STAGE — file code chia theo macro `STAGE`

Mỗi giai đoạn học là một **nhãn** `STAGE` (chỉ là chuỗi nhận diện mode, không phải
giá trị số); đổi nhãn là đổi bài:

```c
/* ==== main.c : chọn bài đang học ==== */
#define STAGE 0_1     /* nhãn chọn mode: 0_1 | 0_2 | 0_3 */

#if   STAGE == 0_1
    /* ... HAL ...         */
#elif STAGE == 0_2
    /* ... CMSIS + raw ... */
#elif STAGE == 0_3
    /* ... raw DSL ...     */
#endif
```

| STAGE | Tên | Nội dung | API | Config IAR |
|---|---|---|---|---|
| **0_1** | DMA thuần — HAL | mem2mem + TIM→DMA→`GPIOC_ODR` (LED PC13) | **HAL** | `Debug_HAL` |
| **0_2** | DMA thuần — CMSIS(+raw) | cùng đề, ưu tiên CMSIS, hạ raw DSL khi buộc | **CMSIS + raw** | `Debug` |
| **0_3** | DMA thuần — raw only | cùng đề, thuần DSL `RSTRUCT`/`BUNION` | **raw** | `Debug` |
| 1 | DMA + poll `TCIF` | (buổi sau) | HAL → raw | — |
| 2 | DMA + interrupt NVIC | (buổi sau) | HAL → raw | — |

**Mục tiêu Buổi 12 = chốt xong 0_1, 0_2, 0_3.** Nút **PA0**, LED **PC13**.
Giai đoạn 0 **được phép** chạm thanh ghi nhưng **cấm** polling & interrupt.

## 3. STAGE 0_1 — DMA thuần bằng **HAL** (kho `No.0_...Draft`)

**Đề bài A (mem2mem):** DMA chép 8 word RAM→RAM, bật xong soi C-SPY (không poll).
**Đề bài B (LED PC13):** `TIM_UP → DMA1_CH5 → GPIOC_ODR`, LED nháy khi CPU đứng im.

```c
/* comments in English — see STANDARD.md */
#include "stm32f1xx_hal.h"                 /* pulls hal_conf -> hal_dma, hal_gpio */

DMA_HandleTypeDef hdma;

void Stage01_MemToMem(uint32_t *src, uint32_t *dst)   /* 8 words, word-size */
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma.Instance                 = DMA1_Channel1;
    hdma.Init.Direction           = DMA_MEMORY_TO_MEMORY;
    hdma.Init.PeriphInc           = DMA_PINC_ENABLE;
    hdma.Init.MemInc              = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_NORMAL;
    hdma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma);
    HAL_DMA_Start(&hdma, (uint32_t)src, (uint32_t)dst, 8);  /* fires now       */
    /* NO poll, NO wait: inspect dst[] + CNDTR in C-SPY dumpRegs               */
}
```

> ✅ **Đã rút ruột file hãng (§N, `git mv` — giữ history):** `stm32f1xx_hal_dma.c`,
> `stm32f1xx_hal_dma.h`, `stm32f1xx_hal_dma_ex.h`, `stm32f1xx_hal_cortex.c/.h` →
> Draft; `stm32f1xx_hal_conf_template.h` → Draft `0_common/include/` đổi tên
> **`stm32f1xx_hal_conf.h`**.
> ⏳ **Còn phải "tailor" `hal_conf.h`:** template bật cả ADC/CAN/SD… (chưa rút) →
> phải **tắt** các module chưa rút, chỉ để `HAL_MODULE / DMA / GPIO / RCC / TIM /
> CORTEX / FLASH / PWR` + set `HSE_VALUE` đúng board; nếu để nguyên sẽ `#include`
> header không tồn tại và fail build. (Đây là lý do `hal_conf.h` **bắt buộc "của
> mình"** — ST cố ý chỉ ship template.)

## 4. STAGE 0_2 & 0_3 — **CMSIS** rồi **raw** (đối chiếu vân tay thanh ghi với 0_1)

**0_2 — cách đọc-dễ (CMSIS, ưu tiên 1)**, chỉ hạ raw DSL ở chỗ cần đóng đinh:

```c
#include "stm32f103xb.h"                   /* CMSIS device header: readable names */

RCC->AHBENR      |= RCC_AHBENR_DMA1EN;
DMA1_Channel1->CPAR  = (uint32_t)src;
DMA1_Channel1->CMAR  = (uint32_t)dst;
DMA1_Channel1->CNDTR = 8;
DMA1_Channel1->CCR   = DMA_CCR_MEM2MEM
                     | DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1   /* 32-bit */
                     | DMA_CCR_MINC    | DMA_CCR_PINC;
DMA1_Channel1->CCR  |= DMA_CCR_EN;         /* fires now — inspect in C-SPY      */
```

**0_3 — cách siêu-chính-xác (raw DSL, ưu tiên 2)**: cùng đề, viết theo
`STANDARD.md` (`RSTRUCT`/`BUNION`), file map mới `dma.h` ở
`C&C++\...\stm32f10x\driver\include\` (**chưa tồn tại → tạo mới**).

Đề bài B (LED, circular) — **on-theme nhất**: `TIM_UP` phát update-event (`UDE`)
→ DMA `CIRC` chép pattern `{0x2000,0x0000}` vào `GPIOC->ODR` → PC13 nháy,
`while(1)` trống. "Timer làm nhịp, DMA tải dữ liệu, CPU đứng ngoài".

## 5. Nghiệm thu 0_1 ↔ 0_2 ↔ 0_3 — "cùng dấu vân tay thanh ghi"

| Bước | Việc |
|---|---|
| 1 | chạy 0_1 (HAL) → C-SPY dumpRegs → fingerprint HAL (`CCR`/`CNDTR`/`CPAR`/`CMAR`/`ISR`) |
| 2 | chạy 0_2 (CMSIS) & 0_3 (raw) → dumpRegs → fingerprint bare-metal |
| 3 | **diff**: ba bản để lại **cùng** giá trị thanh ghi = hiểu đúng |
| 4 | HAL set khác mình chỗ nào (default an toàn? chặn erratum?) → ghi report |

*(Hình minh hoạ: xem mục "Figures" cuối file — bản cũ đã lỗi thời, đang vẽ lại.)*

## 6. Lớp ứng dụng — `dma_fsm_button` / `dma_fsm_led` / `dma_fsm_time`

**Quy tắc đặt tên hàm:** chỉ tiền tố `dma` viết thường; mọi từ sau viết hoa chữ
đầu, ngăn bằng `_` → `dma_<Subsystem>_<Action>_<Action>()`.

**Đánh giá bộ `fsm_*` cũ → thiết kế lại & đổi tên** (giữ tư duy `.h` = khung sườn,
`.c` = trọn trạng thái, `main.c` chỉ gọi ra + chạy loop if/else):

| File cũ | Vướng | File mới |
|---|---|---|
| `fsm_time` | dựa **SysTick-interrupt** | `dma_fsm_time` — timebase **TIM phần cứng** |
| `fsm_button` | chỉ **1 ngưỡng** HOLD; init bằng hàm dài dòng | `dma_fsm_button` — **3 ngưỡng** hold, init bằng **struct** |
| `fsm_led` | thiếu 2 kiểu nháy | `dma_fsm_led` — thêm nháy đều/không đều, drive qua DMA |

### `dma_fsm_time` — thời gian bằng **timer phần cứng**, không SysTick

- Cấu hình một TIM (VD **TIM2**) prescaler để `CNT` tăng 1 / 1 ms; hàm đọc giờ
  = **đọc thẳng `TIM2->CNT`** — không ISR, không spin cờ.
- **Tách hàm đọc giờ riêng cho từng module** (LED và nút nhấn có mốc thời gian
  riêng, dùng if/else bình thường): `dma_Time_Button_Millis()` cho nút,
  `dma_Time_Led_Millis()` cho LED — cùng nền `TIM2` nhưng gọi riêng cho gọn logic.
- **Gọi thư viện TIM theo STAGE:** 0_1 → `stm32f1xx_hal_tim.h` (HAL); 0_2/0_3 →
  CMSIS `TIM2->...` / raw DSL. (Lưu ý: "PWM điều khiển thời gian" ở đây thực chất là
  **TIM đếm làm timebase**; PWM-duty thật chỉ dùng cho độ sáng LED — xem `dma_fsm_led`.)
- ⚠️ *Ghi chú trung thực:* đọc `CNT` là "hỏi giờ", **không** phải poll cờ trong
  vòng lặp → vẫn tôn trọng "no interrupt". Đây là chỗ Stage 0 chạm trần: FSM cần
  một mốc thời gian, ta lấy nó **không qua ngắt**.

```c
void         dma_Time_Init(unsigned int coreHz);   /* TIM2: 1 tick = 1 ms, free-run */
unsigned int dma_Time_Button_Millis(void);         /* mốc thời gian cho nút (TIM2->CNT) */
unsigned int dma_Time_Led_Millis(void);            /* mốc thời gian cho LED (TIM2->CNT) */
void         dma_Time_Delay(unsigned int ms);      /* busy-wait, setup only         */
```

### `dma_fsm_button` — PA0, **init bằng struct** (không hàm init)

- `.h` = **khung**: `enum Button_Event` + `struct dma_Button_Context` (cờ bật từng
  gesture + ngưỡng thời gian + event + state runtime).
- `.c` = khai báo **1 dòng aggregate initializer** cho nút PA0 (điền bit-field cho
  tới khi đầy struct), rồi `dma_Button_Process()` chạy FSM.
- `dma_Button_Hardware_Scan()` = khung **tùy chọn** cho trường hợp **nhiều nút bằng
  mảng** → dự án này **1 nút PA0 nên bỏ**, khai báo trực tiếp.

```c
typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SINGLE, BTN_EVENT_DOUBLE, BTN_EVENT_TRIPLE,
    BTN_EVENT_HOLD_2S, BTN_EVENT_HOLD_5S, BTN_EVENT_HOLD_10S
} Button_Event;

typedef struct {                 /* '.h' = cái khung */
    unsigned int Single_En, Double_En, Triple_En, Hold_En;   /* bật/tắt gesture */
    Button_Event Event;                                      /* output          */
    unsigned int Click_Count;                                /* runtime         */
    unsigned int Gap_Ms;                                     /* cửa sổ gộp click */
    unsigned int Hold2_Ms, Hold5_Ms, Hold10_Ms;             /* 2000/5000/10000  */
} dma_Button_Context;

/* '.c' — init bằng struct, KHÔNG viết hàm init:
 * dma_Button_Context btn_PA0 =
 *     { 1,1,1,1, BTN_EVENT_NONE, 0, 400, 2000,5000,10000 };
 * (so với dự án drone của bạn: DRN_Button_Context drn_btn_PA0 = {1,0,0,0,BTN_EVENT_NONE,0,500}) */

void dma_Button_Process(dma_Button_Context* b, int pressed, unsigned int now);
/* pressed = mức đã CHỐNG RUNG (debounce nằm ở main.c — xem mục 8) */
```

> **Setup GPIO PA0** (clock + input + pull-up): vì Button không có hàm init, đặt
> trong **phần setup của `main.c`** cạnh hàm debounce (raw/HAL theo STAGE).

### `dma_fsm_led` — PC13, soft-PWM nạp qua DMA

> **Sự thật phần cứng:** PC13 là chân TAMPER-RTC, **KHÔNG có kênh PWM phần cứng**
> (smoke-test §16 phải nhảy sang **PB0** cho MODE 3 PWM). ⇒ độ sáng PC13 = **soft-PWM**.
> Cách "công nghiệp" ở đây: **TIM_UP → DMA → `GPIOC_ODR`** tải sẵn pattern; timer
> làm nhịp, DMA làm việc, CPU rảnh — đúng tinh thần "PWM điều khiển thời gian".

```c
typedef enum {
    LED_MODE_OFF = 0, LED_MODE_ON, LED_MODE_TOGGLE,
    LED_MODE_BLINK_EVEN,    /* nháy đều          (hold 2s)  */
    LED_MODE_BLINK_UNEVEN,  /* nháy không đều    (hold 5s)  */
    LED_MODE_FADE_UP,       /* sáng dần 3s, lặp  (double)   */
    LED_MODE_FADE_DOWN      /* tối dần 3s, lặp   (triple)   */
} Led_Mode;

void dma_Led_Hardware_Init(dma_Led_Context* l, volatile GPIO_TypeDef* port,
                           unsigned int pin, unsigned int activeLow);  /* set GPIO */
void dma_Led_Set_On   (dma_Led_Context* l);
void dma_Led_Set_Off  (dma_Led_Context* l);
void dma_Led_Set_Toggle(dma_Led_Context* l);
void dma_Led_Set_Blink_Even  (dma_Led_Context* l, unsigned int periodMs);
void dma_Led_Set_Blink_Uneven(dma_Led_Context* l, unsigned int onMs, unsigned int offMs);
void dma_Led_Set_Fade_Up  (dma_Led_Context* l, unsigned int totalMs);  /* sang dan */
void dma_Led_Set_Fade_Down(dma_Led_Context* l, unsigned int totalMs);  /* toi dan  */
void dma_Led_Process(dma_Led_Context* l, unsigned int now);            /* dùng dma_Time */
```

## 7. Việc chuẩn bị — nhập kho (ĐÃ XONG) + config HAL

**(7a) Rút ruột file hãng — ✅ ĐÃ `git mv`** (xem mục 3): `hal_dma(.c/.h/_ex.h)` +
`hal_cortex(.c/.h)` → Draft `driver`; `hal_conf_template.h` → `0_common/include`
đổi tên `stm32f1xx_hal_conf.h`.

**(7b) Tailor `stm32f1xx_hal_conf.h` — ⏳ còn phải làm** (mục 3): tắt module chưa rút,
để lại DMA/GPIO/RCC/TIM/CORTEX/FLASH/PWR, set `HSE_VALUE`.

**(7c) Config IAR:**

| Track | Config | Symbols | Include thêm | Add source |
|---|---|---|---|---|
| bare-metal (0_2, 0_3) | **`Debug`** | `STM32F103xB` | `C&C++\...` (+ CMSIS device header cho 0_2) | *(raw: không)* |
| HAL (0_1) | **`Debug_HAL`** (base=Debug) | `STM32F103xB`, **`USE_HAL_DRIVER`** | Draft `driver\include` + `0_common\include` + CubeF1 `CMSIS\...\Include` + `CMSIS\Device\...\Include` | `hal.c`,`hal_rcc.c`,`hal_gpio.c`,**`hal_dma.c`**,`hal_cortex.c`,`hal_tim.c` |

Gốc chung `$REPO$ = $PROJ_DIR$\..\..\..\..\..\..` (= `D:\libraries`). Thêm path bằng
**GUI**; nếu sửa `.ewp` tay thì `&` trong `C&C++` phải ghi `&amp;` (file .ewp là XML).

**(7d) printf (yêu cầu 3, hold 10s):** dùng **Semihosting C-SPY** —
`General Options > Library Configuration > Semihosting`. `printf("reset\n")` hiện ở
**Terminal I/O** của IAR khi Download&Debug. Không đụng phần cứng SWD-thuần.

## 8. `main.c` — 6 hành vi (yêu cầu 3)

`main` chỉ chạy setup + loop + if/else. **Debounce là 1 hàm ngay trong `main.c`**
(theo yêu cầu 2) — nó đọc chân RAW PA0 + lọc rung bằng `dma_Time_Button_Millis()`, rồi đưa
mức đã sạch vào `dma_Button_Process()`:

```c
static int debounce_PA0(void)          /* software debounce, dùng dma_Time_Button_Millis() */
{
    /* đọc GPIOA IDR bit0 (raw), lọc ~30 ms như smoke-test §16, trả mức đã sạch */
}

int main(void)
{
    dma_Time_Init(8000000u);           /* TIM2 timebase 1 ms      */
    /* setup GPIO PA0 (input pull-up) ngay đây — Button không có hàm init */
    dma_Led_Hardware_Init(&led, GPIOC, 13u, 1u);

    while (1) {
        dma_Button_Process(&btn_PA0, debounce_PA0(), dma_Time_Button_Millis());
        switch (btn_PA0.Event) {
          case BTN_EVENT_SINGLE:   dma_Led_Set_Toggle(&led);                break; /* 1 */
          case BTN_EVENT_DOUBLE:   dma_Led_Set_Fade_Up(&led, 3000);         break; /* 2 sáng dần 3s */
          case BTN_EVENT_TRIPLE:   dma_Led_Set_Fade_Down(&led, 3000);       break; /* 3 tối dần 3s  */
          case BTN_EVENT_HOLD_2S:  dma_Led_Set_Blink_Even(&led, 400);       break; /* 4 nháy đều    */
          case BTN_EVENT_HOLD_5S:  dma_Led_Set_Blink_Uneven(&led,100,700);  break; /* 5 nháy k.đều  */
          case BTN_EVENT_HOLD_10S: dma_Led_Set_Off(&led); printf("reset\n");break; /* 6 tắt + reset */
          default: break;
        }
        dma_Led_Process(&led, dma_Time_Led_Millis());
    }
}
```

| # | Nút PA0 | LED PC13 |
|---|---|---|
| 1 | single click | toggle on/off |
| 2 | double click | sáng dần trong 3s, lặp liên tục |
| 3 | triple click | tối dần trong 3s, lặp liên tục |
| 4 | hold 2s | nháy đều |
| 5 | hold 5s | nháy không đều |
| 6 | hold 10s | tắt LED + `printf("reset")` ra Terminal I/O |

## 9. Definition of Done (theo `docs/platform-v2.md` mục 3.1)

`Note_12.txt` (user duyệt trước commit) + snapshot `.c.txt` cả 0_1/0_2/0_3 +
`runtime_buoi12.log` + đã `git mv` bộ DMA/cortex vào Draft + đã tailor
`stm32f1xx_hal_conf.h` + `report_buoi_12.md` (bảng so sánh HAL↔CMSIS↔raw: số dòng /
dễ đọc / footprint / vân tay thanh ghi) + figure mới + log bug nếu có +
kiểm B2-pair trước push.

## Figures

Hình cũ (`fig_12_datapath.png`, `fig_12_two_track.png`) **đã lỗi thời** (còn nói tới
track LL) → **sẽ xoá và vẽ lại**:

- `fig_12_dma_datapath.png` — LED PC13 nháy bằng DMA thuần: TIM_UP (UDE) → DMA1_CH5 → `GPIOC_ODR`, CPU đứng ngoài.
- `fig_12_api_ladder.png` — cùng đề đi qua 0_1 (HAL) ↔ 0_2 (CMSIS) ↔ 0_3 (raw), ba vân tay thanh ghi gặp nhau ở C-SPY dumpRegs.

---

### Phụ lục — conflict đã phát hiện & cách xử lý

| # | Conflict | Xử lý |
|---|---|---|
| C1 | Draft thiếu HAL DMA + `hal_conf.h` (+`hal_cortex`) | ✅ đã `git mv`; ⏳ tailor conf (mục 3, 7) |
| C2 | `fsm_time` dùng SysTick | `dma_fsm_time` đọc `TIM2->CNT` (mục 6) |
| C3 | `fsm_button` 1 ngưỡng hold + init dài dòng | `dma_fsm_button` 3 ngưỡng, init bằng struct (mục 6) |
| C4 | PC13 không có PWM phần cứng | soft-PWM + TIM→DMA→ODR (mục 4, 6) |
| C5 | printf vs SWD-thuần | Semihosting C-SPY (mục 7d) |
| C6 | Stage 0 "cấm poll/interrupt" vs FSM 6 hành vi | tách 0_x tối giản ↔ vỏ ứng dụng; ghi rõ chỗ chạm trần (mục 2, 6) |
| C7 | debounce ở main trái luật "main chỉ loop" | chấp nhận theo yêu cầu 2; `Hardware_Scan` chỉ đọc RAW, lọc rung ở main (mục 8) |
