# KI-ENV-0002 — Cùng source y hệt, `RCC_Init` không tìm thấy trên 1 máy

> **BUG MỞ (chưa tái hiện được).** Ghi lại đàng hoàng để tương lai truy ra.

**Triệu chứng:** **source trên PC và laptop GIỐNG HỆT nhau — không sai 1 ký tự**
(đã dò rất kĩ). Vậy mà **PC build được, laptop báo hàm `RCC_Init` "không tìm thấy"**.
Xảy ra thời kỳ đầu học (giai đoạn `Buoi_4`), khi `rcc.h` là file cấp clock cho MCU.
Giờ project đã đổi quá nhiều nên **chưa tái hiện lại được**.

## Điểm mấu chốt
Vì source **y hệt** trên 2 máy, nguyên nhân **KHÔNG nằm trong source code**. Nghi ở:
- **Artifact biên dịch cũ/bẩn**: `.o` trong `Debug/Obj/`, `.exe`/output, cache build.
- **File cấu hình/project của IAR khác nhau giữa 2 máy**: `.ewp` / `.ewd` / `.dni` /
  thư mục `settings/`, hoặc **khác phiên bản EWARM**.

Tức `origin = host-environment` (khác máy/artifact), có thể kèm `toolchain` (cấu hình
IAR), **không** phải logic code của mình.

## Vị trí code (hiện tại) & lịch sử tiến hoá
**Chứng cứ chính — bug + fix cạnh nhau:** `Take note quá trình học thanh ghi/Buoi_4/Note.txt`
dòng 13 (trước) và 15 (sau):
```c
void RCC_Init(volatile RCC_TypeDef* RCC, const RCCInit_TypeDef* RCCInit);  // TRƯỚC (dòng 13)
void RCC_Init(volatile RCC_TypeDef* rcc, const RCCInit_TypeDef* RCCInit);  // SAU  (dòng 15)
```
Note.txt cũng ghi: đổi object `@address` → `#define` cho `GPIOB`/`RCC`, thêm `startup_stm32f10x_md.s`.

**Lịch sử tham số hàm qua các buổi:**
- Bản LỖI `RCC` (HOA): `Buoi_1/rcc.h:517`, `Buoi_2/rcc.h:517`,
  `Buoi_3/Blink 1 led/rcc.h:517`, `Buoi_3/Blink n led/rcc.h:517`.
- Nơi ghi FIX: `Buoi_4/Note.txt` (dòng 13 → 15).
- Bản ĐÃ SỬA `rcc` (thường) từ Buoi_5 trở đi: `Buoi_5/Buoi_5.1/rcc.h:520`,
  `Buoi_7/Helper file/rcc.h:520`, `Buoi_8/Helper file/rcc.h:520`,
  `Buoi_9/Helper_files/driver/include/rcc.h:524`.

## Ứng viên nguyên nhân (CHƯA xác nhận)
Một khả năng **source-level**: `RCC` vừa là tham số hàm (HOA) vừa là macro
`#define RCC (*(volatile RCC_TypeDef*)0x40021000)`. Preprocessor (phân biệt hoa/thường)
sẽ bung macro vào chỗ tham số cùng tên → khai báo `RCC_Init` hỏng.
**NHƯNG** giả thuyết này **không giải thích được "khác máy"**: nếu source y hệt thì
lỗi phải xảy ra **giống nhau trên cả 2 máy**. Vì vậy đây chỉ là **ứng viên**, không
phải kết luận. Nghi chính vẫn là **cache build bẩn** hoặc **cấu hình IAR khác nhau**.

## Hướng truy khi tái hiện được
1. `Project → Clean`, rồi `Rebuild All`; **xoá thủ công `Debug/Obj/`**.
2. So sánh `.ewp` / `.ewd` / `settings/` giữa 2 máy (diff).
3. Kiểm tra **phiên bản EWARM** 2 máy có khác nhau không.
4. Kiểm tra có object `.o` cũ (biên dịch từ source trước đó) còn sót lại không.
5. Bật xem **preprocessed output** (`.i`) để chắc chắn `RCC_Init` khai báo đúng sau macro.

## Cách sửa
Chưa có cách sửa xác nhận cho phần "khác máy" (bug mở). Thay đổi từng ghi trong
`Buoi_4/Note.txt` (`RCC`→`rcc`, object→`#define`) đã được áp dụng ở giai đoạn đó,
nhưng **chưa chắc** là nguyên nhân/cách sửa của lỗi khác-máy.

## Bài học tạm
- Khi **2 máy cùng source mà khác kết quả build** → nghi **artifact/cache bẩn** hoặc
  **cấu hình/phiên bản IAR khác nhau**, đừng chỉ soi source.
- Tiện thể: tránh đặt **tham số/biến trùng tên macro ngoại vi** (`RCC`, `GPIOB`…)
  để loại luôn giả thuyết macro-collision.

## Trạng thái
`open` — chưa tái hiện, chưa xác nhận nguyên nhân. Cần **log lỗi nguyên văn trên
laptop** (nếu tìm lại được) để điền `error.log` + `codes`.
