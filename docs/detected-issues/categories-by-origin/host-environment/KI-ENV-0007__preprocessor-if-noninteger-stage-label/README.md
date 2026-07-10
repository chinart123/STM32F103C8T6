# KI-ENV-0007 — `#if STAGE == 0_1` với nhãn `0_1/0_2/0_3` → Pe014 (nhãn không phải số nguyên)

**Triệu chứng:** build config `DMA_B12_S01_HAL` của `main_buoi12_dma.c` **FAIL ngay ở
tiền xử lý**, 3 lần cùng một lỗi tại các dòng `#if`/`#elif`, kèm 1 lỗi `#error` rơi xuống `#else`:
```
main_buoi12_dma.c(22) : Error[Pe014]: extra text after expected end of preprocessing directive
main_buoi12_dma.c(67) : Error[Pe014]: extra text after expected end of preprocessing directive
main_buoi12_dma.c(72) : Error[Pe014]: extra text after expected end of preprocessing directive
main_buoi12_dma.c(78) : Fatal Error[Pe035]: #error directive: "No STAGE selected ..."
Total number of errors: 4
```
Xem `error.log` (nguyên văn iarbuild, đã tái hiện lại có chủ đích).

> **Đây KHÔNG phải lỗi thiết kế của người dùng.** Lược đồ nhãn `0_1 / 0_2 / 0_3`
> (đặt tên giai đoạn học) là hoàn toàn hợp lý. Lỗi nằm ở chỗ **AI hiện thực sai**:
> đem một *nhãn-chuỗi* nhét vào `#if ==` — cơ chế chỉ dành cho **số nguyên**. Vì thế
> xếp vào kho hệ thống (origin `language` + `host-environment`), không phải `bug_log` firmware-logic.

## AI đã làm gì (bản SAI)

```c
#define STAGE 0_1        /* ý đồ: coi 0_1/0_2/0_3 là "giá trị" của STAGE */

#if   STAGE == 0_1       /* HAL   */
#elif STAGE == 0_2       /* CMSIS */
#elif STAGE == 0_3       /* raw   */
#else
#  error "No STAGE selected ..."
#endif
```

## Chuỗi nguyên nhân — vì sao sai

1. `#if` của tiền xử lý C chỉ ước lượng được **biểu thức hằng số nguyên** (integer
   constant expression — ISO C §6.10.1). Sau khi bung macro, `#if STAGE == 0_1` trở thành
   `#if 0_1 == 0_1`.
2. Token `0_1` là một **pp-number** (ISO C §6.4.8): hợp lệ về *từ vựng* nên trình biên
   dịch không kêu "ký tự lạ", nhưng nó **không phải** một hằng số nguyên hợp lệ — một
   chuỗi chữ số có ký tự `_` xen giữa không tạo thành literal số nào.
3. Bộ tiền xử lý đọc được số `0`, rồi thấy `_1` **thừa ra** sau chỗ lẽ ra directive phải
   kết thúc → phát `Error[Pe014]: extra text after expected end of preprocessing directive`.
4. Cả ba nhánh `#if`/`#elif` hỏng y hệt ⇒ bộ tiền xử lý coi như **không nhánh nào đúng**
   ⇒ rơi xuống `#else` ⇒ `Fatal Error[Pe035]` "No STAGE selected".

> Bẫy tinh vi: `#if UNDEFINED_NAME` thì tên chưa định nghĩa được thay bằng `0` (không lỗi),
> nên người ta dễ tưởng `#if` "chấp nhận mọi thứ". Nhưng một **pp-number sai** như `0_1`
> thì KHÁC — nó không bị thay bằng 0, mà làm hỏng cả directive.

## Vị trí code (hiện tại — bản ĐÃ SỬA)

`Manufacturer_Package/No.0_..Draft/.../1_Application/Buoi_12/main_buoi12_dma.c` (dòng 17-25, 25/70/75):
```c
/* Pick the mode: define exactly ONE of these labels (comment out the others).
 * The label name IS the mode tag - no numbers, tested with #ifdef (not #if ==). */
#define STAGE_0_1        /* 0_1 = HAL   (this build)   */
/* #define STAGE_0_2 */  /* 0_2 = CMSIS (+ raw)        */
/* #define STAGE_0_3 */  /* 0_3 = raw DSL only         */
...
#ifdef STAGE_0_1
#elif defined(STAGE_0_2)
#elif defined(STAGE_0_3)
#else
#  error "No STAGE selected - define STAGE_0_1 / STAGE_0_2 / STAGE_0_3"
#endif
```

## Cách sửa (đã áp dụng)

- Biến **mỗi nhãn thành một macro độc lập** (`STAGE_0_1`, `STAGE_0_2`, `STAGE_0_3`), chọn
  bài bằng cách **định nghĩa đúng một** cái; kiểm tra bằng `#ifdef` / `#elif defined(...)`
  thay cho `#if ==`.
- Build lại config `DMA_B12_S01_HAL`: **Total number of errors: 0 / warnings: 0**, Linking OK,
  đã chạy PASS trên board (xem `report_Buoi_12_Problem_A`).

## Bài học / phòng ngừa

- **`#if` chỉ so sánh SỐ NGUYÊN.** Muốn "chọn 1 trong nhiều nhãn dạng chữ" → dùng
  `#ifdef` / `defined()`, mỗi nhãn một macro. Không bao giờ đặt một nhãn kiểu `0_1` sau `#if ==`.
- Nhãn `0_1` trông như số nhưng là **pp-number**, không phải `int` literal → không dùng được
  trong số học tiền xử lý.
- **Trách nhiệm dịch ý người dùng sang cơ chế đúng là của người viết code.** Người dùng chọn
  tên `0_1/0_2/0_3` là hợp lệ; việc cần làm là ánh xạ nó sang `#ifdef`, chứ không ép `==`.

## Trạng thái
`fixed` — đã đổi sang `#ifdef`, build 0/0 và chạy PASS trên board. Tham chiếu chéo:
`Log-and-Report-writing-tools/logs/bug_log.md` (bản ghi local cùng bug).
