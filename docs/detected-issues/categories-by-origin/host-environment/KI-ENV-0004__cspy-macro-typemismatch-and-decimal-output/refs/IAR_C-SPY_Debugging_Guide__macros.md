# Tài liệu tham chiếu — IAR C-SPY® Debugging Guide (macro system)

**Định danh:** IAR C-SPY Debugging Guide for Arm — chương *"The C-SPY macro system"*
(mục *Reference information on the macro language* và *C-SPY system macros*:
`__openFile`, `__closeFile`, `__message`, `__fmessage`, `__readMemory32`).
Tra khớp bản EW 8.3 đang dùng.

## Trích đoạn liên quan (đặc tả)

### `__openFile(filename, access)` — trả về FILE-HANDLE, không phải int
- Tham số `access` là chuỗi: `"r"` đọc, `"w"` ghi, `"a"` ghi nối (append). `"a"`/`"w"`
  **tạo file nếu chưa có** → đó là lý do file `runtime.log` xuất hiện dù chưa ghi gì.
- Giá trị trả về là **một macro variable kiểu định danh file của C-SPY** — dùng để
  test theo kiểu boolean (`if (handle) …`). Ví dụ mẫu trong tài liệu:
  ```
  _fileHandle = __openFile("$PROJ_DIR$\\...\\out.log", "a");
  if (_fileHandle)          // <-- test truthiness, KHÔNG so == 0
  {
      __fmessage _fileHandle, "...";
  }
  ```
- ⇒ **Liên hệ bug (Lỗi A):** viết `if (f == 0)` là đem file-handle so `==` với `int` →
  **Type mismatch**. Đúng phải `if (f) { … } else { … }`.

### `__message` / `__fmessage` — radix mặc định là THẬP PHÂN, format bằng `:%fmt`
- `__message argList;` in ra Log window; `__fmessage fileHandle, argList;` in ra file.
- Mỗi đối số số học in ở **hệ thập phân** theo mặc định.
- Có thể gắn **format specifier** sau biểu thức bằng dấu hai chấm: `expr:%X`.
  Các format thường dùng: `%d` thập phân, `%x`/`%X` thập lục, `%o` bát phân, `%b` nhị phân,
  `%c`, `%s`. Ví dụ: `__fmessage f, "ISER0=0x", value:%x, "\n";`
- ⇒ **Liên hệ bug (Lỗi B):** thiếu `:%x` nên thanh ghi in thập phân (`31` thay vì `0x1F`).

### `__readMemory32(address, zone)`
- Đọc 32-bit tại `address` trong `zone` (vd `"Memory"`). Cần phiên debug đang chạy và
  (để giá trị ổn định) core đã halt. Trả về giá trị số → phải format `:%x` nếu muốn HEX.

## Snippet + vị trí trong repo
Bản đã sửa áp đúng cả 2 điều trên:
`Log-and-Report-writing-tools/skills/runtime-capture/capture.mac`
(hàm `dumpRegs()` và `dumpTim1()` — dùng `if (f)` + `v:%x`).

## Ghi chú
Tên hàm/đặc tả có thể lệch nhẹ giữa các bản EWARM; nếu một tên khác trên máy khác thì
tra lại đúng chương macro của bản đó. Hai nguyên tắc cốt lõi (file-handle test bằng
`if(handle)`, số in thập phân trừ khi có `:%x`) là ổn định qua các bản.
