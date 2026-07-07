# ISO/IEC 9899 — Programming languages — C

> Tài liệu cho **ứng viên nguyên nhân** (macro-collision). Không phải nguyên nhân
> đã xác nhận của lỗi "khác máy" — xem README.

## 1. Định danh tài liệu
- **Tên đầy đủ:** ISO/IEC 9899 — Information technology — Programming languages — C
- **Nhà phát hành:** ISO/IEC (bản làm việc miễn phí: WG14, vd N1256 cho C99, N1570 cho C11)
- **Phiên bản:** C99 / C11 (điền bản áp dụng; IAR EWARM 8.x mặc định C11/C99)
- **Ngày phát hành:** C99 = 1999; C11 = 2011 (bản dự thảo N1570: 2011-04-12)
- **Ngày sửa đổi (nếu có):** —
- **Link:** open-std.org/JTC1/SC22/WG14 (dự thảo công khai)
- **Mục/section liên quan:** §6.10.3 *Macro replacement* (và §6.4.2 *Identifiers* — case-sensitive)
- **Trích đoạn (nguyên văn):**
  > «(dán nguyên văn §6.10.3). Ý chính: tên macro giống-đối-tượng, khi xuất hiện,
  > được thay bằng chuỗi thay thế; định danh trong C phân biệt hoa/thường, nên chỉ
  > token trùng CHÍNH XÁC tên macro mới bị thay.»

## 2. Liên hệ tới bug của mình
Nếu `RCC` là macro `#define RCC (*(volatile RCC_TypeDef*)0x40021000)`, thì token
`RCC` dùng làm **tên tham số** trong `void RCC_Init(volatile RCC_TypeDef* RCC, …)`
cũng bị thay → khai báo hỏng. Vì macro **case-sensitive**, đổi tham số thành `rcc`
(chữ thường) sẽ **không** bị thay. Đây giải thích được *vì sao đổi `RCC`→`rcc` giúp
biên dịch*, **nhưng không** giải thích *vì sao 2 máy cùng source lại khác nhau*.

## 3. Snippet + vị trí (hiện tại)
Chứng cứ chính (bug + fix cạnh nhau): `Take note quá trình học thanh ghi/Buoi_4/Note.txt`
(dòng 13 → 15):
```c
void RCC_Init(volatile RCC_TypeDef* RCC, const RCCInit_TypeDef* RCCInit);  // TRƯỚC (dòng 13)
void RCC_Init(volatile RCC_TypeDef* rcc, const RCCInit_TypeDef* RCCInit);  // SAU  (dòng 15)
//                                  ^^^ tham số trùng tên macro RCC (nếu RCC là #define)
```
Bản LỖI (`RCC` HOA) còn nguyên ở `Buoi_1/rcc.h:517`, `Buoi_2/rcc.h:517`,
`Buoi_3/{Blink 1 led,Blink n led}/rcc.h:517`; bản đã sửa (`rcc`) từ `Buoi_5/Buoi_5.1/rcc.h:520`.
