# KI-ENV-0004 — `capture.mac` sai cú pháp C-SPY → log rỗng; và quên format HEX

**Triệu chứng:** chạy macro C-SPY `dumpTim1()` (đọc thanh ghi qua SWD ghi ra
`runtime.log`) thì **không thấy nội dung** — file `runtime.log` **có được tạo nhưng 0 byte**.
Debug Log của IAR spam liên tục:
```
capture.mac(48,11): Error: Type mismatch.
```
Sau khi qua được lỗi đó thì log lại **in số THẬP PHÂN** (`NVIC_ISER0=0`, `TIM1_SR=31`)
thay vì HEX, khó đối chiếu thanh ghi.

Đây là **2 lỗi trong script host của mình** (`capture.mac`), lộ ra do **đặc tả ngôn ngữ
macro C-SPY của IAR**, không phải bug firmware.

## Chuỗi nguyên nhân

### Lỗi A — `if (f == 0)` gây "Type mismatch" → macro chết, để lại file rỗng
1. `f = __openFile(path, "a");` — trong ngôn ngữ macro C-SPY, `__openFile` trả về **một
   giá trị kiểu FILE-HANDLE riêng**, KHÔNG phải số nguyên.
2. Chế độ `"a"` khiến `__openFile` **tạo file ngay** (nên `runtime.log` xuất hiện).
3. Dòng kế `if (f == 0)` đem **file-handle so `==` với số nguyên `0`** → **sai kiểu** →
   C-SPY báo `Type mismatch` tại `(48,11)` (đúng cột toán tử `==`) và **dừng macro**.
4. Vì dừng ngay sau `__openFile`, chưa kịp `__fmessage`/`__closeFile` → file **0 byte**.

### Lỗi B — `__fmessage` in thập phân theo mặc định
1. `__fmessage f, "NAME=", __readMemory32(addr,"Memory"), "\n";` — `__fmessage`/`__message`
   **in số ở hệ THẬP PHÂN** theo mặc định.
2. Với thanh ghi (bit-field) thì thập phân gần như vô dụng để debug.
3. Muốn HEX phải **gắn format** `biểu_thức:%x`.

## Vị trí code (hiện tại)
`Log-and-Report-writing-tools/skills/runtime-capture/capture.mac` — bản ĐÃ SỬA:
```c
// __openFile trả về FILE-HANDLE, không phải int -> test bằng if (f), KHÔNG if (f == 0).
// In HEX bằng format v:%x (mặc định là thập phân).
f = __openFile("...runtime.log", "a");
if (f)
{
    v = __readMemory32(0xE000E100, "Memory"); __fmessage f, "NVIC_ISER0=0x", v:%x, "\n";
    ...
    __closeFile(f);
}
else { __message "dumpTim1: cannot open runtime.log"; }
```

## Cách sửa (đã áp dụng)
- **Lỗi A:** đổi `if (f == 0) { ...; return; }` → `if (f) { ... } else { ... }`
  (test file-handle theo kiểu boolean, đúng ví dụ trong tài liệu C-SPY).
- **Lỗi B:** đọc giá trị vào biến `v` rồi `__fmessage f, "NAME=0x", v:%x, "\n";`.
- Sau sửa: chạy lại ra HEX đúng (`NVIC_ISER0=0x02000000`, `TIM1_SR=0x1E`).

## Bài học / phòng ngừa
- Trong macro C-SPY: **file-handle ≠ int**. Kiểm tra mở file thành công bằng `if (handle)`,
  đừng bao giờ `handle == 0`.
- `__message`/`__fmessage` **mặc định in thập phân**; với thanh ghi luôn dùng `:%x`.
- Một macro chết giữa chừng vẫn có thể **để lại file rỗng** (vì `__openFile "a"` tạo file
  trước khi lỗi xảy ra) → "có file 0 byte" là dấu hiệu *mở được nhưng ghi hỏng*, không
  phải *sai đường dẫn*.

## Trạng thái
`fixed` — đã sửa `capture.mac`, người dùng chạy lại xác nhận `runtime.log` ra HEX đúng.
