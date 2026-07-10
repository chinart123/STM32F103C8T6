[[TOC]]

## 1. Tóm tắt điều hành

Buổi 12 — **Đề A**: dùng **DMA** chép 8 word RAM→RAM trên STM32F103C8T6 (bare-metal,
IAR EWARM 8.3), theo tầng API **HAL** (nấc 0_1 của thang HAL → CMSIS → raw). Mục tiêu:
bật "băng chuyền" DMA rồi để CPU đứng ngoài — **không polling, không ngắt** — và chứng
minh kết quả bằng **vân tay thanh ghi** đọc qua C-SPY.

**Kết quả: PASS trên board.** DMA sao chép đúng (`g_dst == g_src`), cờ hoàn tất
`TCIF1 = 1`, `CNDTR = 0`. Đáng chú ý về mặt kỹ thuật: Đề A chỉ chạm **~3%** bề mặt
thư viện hãng (1/≈30 module HAL, đúng 2 hàm) — HAL chỉ đóng vai "công tắc mồi", phần
hiểu nằm ở tầng thanh ghi.

| Hạng mục | Kết quả |
|---|---|
| Build (`iarbuild`, config `DMA_B12_S01_HAL`) | 0 lỗi, 0 warning |
| Chạy trên board (ST-Link V2, SWD) | PASS |
| Sao chép RAM→RAM | `g_dst == g_src` (0x11111111 … 0x88888888) |
| Cờ hoàn tất | `ISR.TCIF1 = 1`, `CNDTR = 0` |
| Phụ thuộc API hãng | ~3% HAL (2 hàm) |

## 2. Chiến lược 3 tầng API (HAL → CMSIS → raw)

Buổi 12 đi một **thang ưu tiên**: **HAL** (nấc 0_1, đơn giản nhất, hợp người mới) →
**CMSIS** (nấc 0_2, tên thanh ghi dễ đọc) → **raw DSL** (nấc 0_3, siêu chính xác theo
`STANDARD.md`). Cùng một đề bài đi qua cả ba, rồi **đối chiếu vân tay thanh ghi**: nếu
ba cách gõ để lại cùng giá trị `CCR/CNDTR/CPAR/CMAR`, tức chúng là **một** ở mức phần
cứng. Báo cáo này là nấc **0_1 (HAL)**.

![Thang API Buổi 12: một đề đi qua 0_1 HAL / 0_2 CMSIS / 0_3 raw, ba cách gõ gặp nhau ở cùng vân tay thanh ghi (C-SPY dumpRegs)](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_api_ladder.png)

## 3. Bản đồ Buổi 12 — Đề A đang ở đâu, Đề B là gì

Buổi 12 có **hai đề bài DMA**, mỗi đề đi qua cả ba nấc thang API ở mục 2:

- **Đề A — mem2mem (báo cáo này).** DMA1 Channel 1 chép 8 word RAM→RAM. Không có
  nhịp bên ngoài, chạy ở tốc độ bus — dùng để **làm quen cơ chế "băng chuyền"** và
  tập đọc vân tay thanh ghi.
- **Đề B — LED PC13 (bước kế tiếp).** `TIM_UP → DMA1 Channel 5 (circular) →
  GPIOC_ODR`: timer làm nhịp để DMA đẩy vòng pattern `{0x2000, 0x0000}` ra chân
  LED, CPU đứng ngoài. Đề B mới cho thấy DMA **được định nhịp** — nhìn `CNDTR` đếm
  lùi theo từng update-event của timer, thứ mà Đề A chạy quá nhanh để quan sát.

Sau khi cả hai đề chốt xong ở cả ba nấc, chúng ghép lên **lớp ứng dụng** (nút PA0
điều khiển LED PC13 qua 6 hành vi). Báo cáo này phủ đúng ô **★** trong bản đồ dưới
đây: *(Đề A, nấc 0_1 HAL)*.

![Bản đồ Buổi 12: hai đề bài (A = mem2mem, B = LED PC13) × ba nấc API (HAL / CMSIS / raw); ô ★ đánh dấu phạm vi báo cáo này = Đề A nấc HAL, các ô còn lại là việc kế tiếp](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_roadmap_overview.png)

## 4. Kiến trúc Đề A — DMA mem-to-mem

DMA là **băng chuyền**: ta khai báo trước nguồn (`CPAR`), đích (`CMAR`), số lượng
(`CNDTR`), cỡ ô + chế độ (`CCR`), bấm `EN`, rồi phần cứng tự chép. CPU **đứng ngoài
hoàn toàn** — `while(1)` trống.

![Kiến trúc Đề A: RAM g_src → DMA1 Channel 1 (MEM2MEM, word, tự tăng địa chỉ) → RAM g_dst; CPU while(1) đứng ngoài luồng dữ liệu](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_A_mem2mem.png)

## 5. Hiện thực

### 5.1 Mã nguồn (`main_buoi12_dma.c`)

Một `main.c` duy nhất, chọn tầng API bằng nhãn `#ifdef STAGE_0_x` (nhãn là *tên macro*,
không phải số — nên kiểm tra bằng `#ifdef`, không dùng `#if ==`). Phần cốt lõi:

```c
static void Stage01_MemToMem(uint32_t *src, uint32_t *dst)
{
    __HAL_RCC_DMA1_CLK_ENABLE();                        /* bật clock DMA1        */
    hdma.Instance                 = DMA1_Channel1;
    hdma.Init.Direction           = DMA_MEMORY_TO_MEMORY;
    hdma.Init.PeriphInc           = DMA_PINC_ENABLE;    /* nguồn tự tăng         */
    hdma.Init.MemInc              = DMA_MINC_ENABLE;    /* đích tự tăng          */
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;/* 32-bit                */
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_NORMAL;         /* one-shot              */
    hdma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma);
    HAL_DMA_Start(&hdma, (uint32_t)src, (uint32_t)dst, 8u);  /* chạy ngay        */
    /* KHÔNG poll, KHÔNG ngắt: đọc g_dst + CNDTR trong C-SPY */
}
```

### 5.2 Cấu hình IAR

Tạo config **`DMA_B12_S01_HAL`** (base = `Debug`): thêm cờ `USE_HAL_DRIVER`, giữ
`STM32F103xB`, bỏ `USE_FULL_LL_DRIVER`. Danh sách biên dịch tối giản: `main_buoi12_dma.c`
+ `stm32f1xx_hal_dma.c` + `startup` + `cortex_m3.c` — loại toàn bộ file LL/smoke-test
khỏi config này (`<excluded>`), nên bản LL `Debug` cũ **không hề đụng**. Bộ HAL tối
thiểu: **không cần** `hal.c`/`system_stm32f1xx.c` (startup không gọi `SystemInit`;
linker tự loại phần HAL không dùng tới).

## 6. Kết quả on-board (bằng chứng)

Nạp bằng ST-Link V2 (SWD thuần), chạy qua `Stage01_MemToMem()`, Halt, đọc C-SPY.

### 6.1 Vân tay thanh ghi DMA1 · Channel 1

*Bảng 1 — thanh ghi kênh 1 sau khi chạy:*

| Thanh ghi | Giá trị | Ý nghĩa |
|---|---|---|
| DMA1_ISR | 0x00000007 | GIF1+TCIF1+HTIF1 → **TCIF1 = 1** (hoàn tất) |
| DMA1_CCR1 | 0x00004AC1 | cấu hình kênh (giải mã Bảng 2) |
| DMA1_CNDTR1 | 0x00000000 | còn 0 word → đã chuyển hết 8 |
| DMA1_CPAR1 | 0x20000000 | nguồn = `&g_src[0]` |
| DMA1_CMAR1 | 0x20000064 | đích = `&g_dst[0]` |

*Bảng 2 — giải mã `CCR1 = 0x00004AC1` (bit-field đọc thẳng từ C-SPY):*

| Bit-field | Giá trị | Nghĩa |
|---|---|---|
| MEM2MEM | 1 | chép bộ nhớ → bộ nhớ |
| MSIZE / PSIZE | 0b10 / 0b10 | ô 32-bit (word) cả hai đầu |
| MINC / PINC | 1 / 1 | cả hai địa chỉ tự tăng |
| CIRC | 0 | one-shot |
| TCIE / HTIE / TEIE | 0 | tắt hết ngắt (đúng luật Stage 0) |
| EN | 1 | kênh đang bật |

→ khớp 100% với `hdma.Init` trong mã HAL.

![Cửa sổ Registers (C-SPY) — nhóm DMA1 kênh 1 mở rộng từng bit-field: CCR1=0x4AC1 (MEM2MEM=1, MSIZE=PSIZE=0b10 tức word, MINC=PINC=1, EN=1), CNDTR1=0, CPAR1=0x20000000, CMAR1=0x20000064 — khớp 100% Bảng 1 và Bảng 2](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/thanh ghi day du -part2.png)

### 6.2 Kiểm chứng bộ nhớ

*Bảng 3 — `g_dst` có bằng `g_src` không (Live Watch, hệ hex):*

| idx | g_src | g_dst | idx | g_src | g_dst |
|---|---|---|---|---|---|
| [0] | 0x11111111 | 0x11111111 | [4] | 0x55555555 | 0x55555555 |
| [1] | 0x22222222 | 0x22222222 | [5] | 0x66666666 | 0x66666666 |
| [2] | 0x33333333 | 0x33333333 | [6] | 0x77777777 | 0x77777777 |
| [3] | 0x44444444 | 0x44444444 | [7] | 0x88888888 | 0x88888888 |

![Trước khi chạy: g_dst[] toàn 0, g_src[] đã có dữ liệu](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/Truoc khi bam run-so sanh 2 mang.png)

![Sau khi chạy: g_dst[] đã bằng g_src[] — DMA sao chép thành công (IAR mặc định hiển thị thập phân: 286331153 = 0x11111111)](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/sau khi bam run-so sanh 2 mang.png)

![Cùng dữ liệu, đổi hiển thị sang hệ hex (chuột phải → Format → Hexadecimal): g_dst[] == g_src[] = 0x11111111 … 0x88888888 — đọc thẳng ra pattern thay vì phải nhẩm số thập phân](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/sau khi bam run-so sanh 2 mang-hexa.png)

## 7. Bài học kỹ thuật

- **mem2mem quá nhanh để nhìn `CNDTR` đếm lùi.** 8 word rip qua ở tốc độ bus (<1µs) —
  mọi lần Halt đều đã thấy `CNDTR = 0`. Bằng chứng "đã chạy" = `CNDTR=0` + `TCIF1=1` +
  `g_dst` đầy. Muốn *tận mắt* thấy đếm lùi → Đề B (TIM→DMA, mỗi bước chờ một nhịp timer).
- **IAR Live Watch mặc định hệ thập phân** (`286331153` = `0x11111111`); đổi hex qua
  chuột phải → Format → Hexadecimal.
- **Cửa sổ Registers phải refresh** (click một thanh ghi) mới hiện giá trị thật.
- **`#if` của tiền xử lý chỉ so sánh số nguyên** — nhãn kiểu `0_1` phải chọn bằng
  `#ifdef`, không dùng `#if ==` (đã sửa, ghi ở `bug_log`).
- **"Configuration" của IAR nằm trong cả `.ewp` lẫn `.ewd`;** tạo config = việc GUI
  (chạm device/linker); danh sách source chia theo config bằng cờ `<excluded>`.

## 8. Mức độ sử dụng API hãng

Đề A cố tình dùng HAL ở mức **tối thiểu** — như một *công tắc mồi* để lên sóng nhanh,
rồi (ở nấc 0_2/0_3) nghiệm thu xuống tận thanh ghi. Điều này thể hiện chủ đích **không
bị khoá vào thư viện hãng** và **hiểu tới tận bit**.

![Mức độ dùng API hãng trong Đề A: ở mức module chỉ 1/≈30 module HAL (~3%, riêng DMA); ở mức hàm chỉ HAL_DMA_Init + HAL_DMA_Start + 1 macro clock. Triết lý: HAL để mồi, nghiệm thu xuống CMSIS/raw](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_api_usage.png)

## 9. Phụ lục — Kiến thức thêm: vì sao cột "Location" cố định giữa các lần chạy

Khi soi Live Watch nhiều lần (Reset → Run → Reset → Run), có thể để ý một điều: cột
**Value** của `g_src`/`g_dst` thay đổi (0 → dữ liệu đã chép), nhưng cột **Location**
(địa chỉ) thì **y hệt mỗi lần**: `g_src @ 0x20000000`, `g_dst @ 0x20000064`.

Lý do: `g_src` và `g_dst` là **biến toàn cục `static`** → **linker** đặt chúng vào vùng
RAM cố định (`.data`/`.bss`) **ngay lúc build**, không nằm trên stack. Mỗi lần
Reset/Download, IAR nạp lại **đúng cùng một ảnh nhị phân** → cùng bố cục bộ nhớ → **cùng
địa chỉ**. Vì thế chỉ **Value** đổi, còn **Location** đứng yên.

Ngược lại, nếu chúng là **biến cục bộ** (trên stack), địa chỉ do con trỏ ngăn xếp (SP)
quyết định lúc chạy và **có thể xê dịch** giữa các lần gọi hay các lần build khác nhau —
khi đó cột Location sẽ không còn cố định.

![Vì sao cột Location cố định: g_src/g_dst là biến static toàn cục, linker đặt tĩnh ở .data/.bss (0x20000000 và 0x20000064) — mỗi lần chạy nạp lại cùng ảnh nhị phân nên địa chỉ không đổi, chỉ Value thay đổi; còn biến cục bộ trên stack thì địa chỉ có thể xê dịch](D:/libraries/Take note quá trình học thanh ghi/Buoi_12/assets/fig_12_static_addr.png)

[[LOF]]
