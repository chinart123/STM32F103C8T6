# IAR C/C++ Development Guide for Arm

## 1. Định danh tài liệu
- **Tên đầy đủ:** IAR C/C++ Development Guide — Compiling and Linking, for Arm Limited's Arm® Cores
- **Nhà phát hành:** IAR Systems AB
- **Phiên bản:** bản đi kèm IAR Embedded Workbench for Arm 8.x (điền số hiệu chính xác theo bản đang cài, vd `EWARM_DevelopmentGuide.ENU`)
- **Ngày phát hành:** (điền theo trang bìa của bản guide đang cài)
- **Ngày sửa đổi (nếu có):** (điền theo trang "Revision history" của guide)
- **Link:** Help → C/C++ Development Guide trong IAR EW (hoặc thư mục cài `...\arm\doc\`)
- **Mục/section liên quan:** *"Include file search procedure"* (chương The preprocessor / Compiler options `-I`)
- **Trích đoạn (nguyên văn):**
  > «(dán nguyên văn đoạn mô tả thứ tự tìm header). Ý chính: với `#include "file"`,
  > trình biên dịch tìm theo thứ tự — thư mục chứa file nguồn đang include → các thư
  > mục khai báo bằng `-I` → thư mục header chuẩn của trình biên dịch.»

## 2. Liên hệ tới bug của mình
Vì `time.h` của mình **không** nằm ở thư mục nguồn cũng **không** có trong đường `-I`,
bước cuối của quy trình trên tìm thấy **`<time.h>` chuẩn** trong thư mục header của
compiler và **mở nó**. Header chuẩn không có `Time_Init`/`Time_Millis` → khai báo
ngầm (`Pe223`) → thiếu định nghĩa khi link (`Li005`). Đổi tên thành `fsm_time.h`
khiến không còn header chuẩn nào trùng tên để "chen vào".

## 3. Snippet + vị trí (hiện tại)
`C&C++/Embedded_C99/Microcontroller/stm32f10x/test_lib/stm32f103c8t6/source/main.c` (dòng ~21):
```c
#include "time.h"       // BUG (cũ): resolve nhầm sang <time.h> chuẩn
#include "fsm_time.h"   // FIX hiện tại
```
