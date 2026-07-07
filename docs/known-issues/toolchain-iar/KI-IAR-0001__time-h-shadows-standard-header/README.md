# KI-IAR-0001 — `time.h` cục bộ bị header chuẩn `<time.h>` che mất

**Triệu chứng:** khi build, 2 cảnh báo `Pe223 "declared implicitly"` cho `Time_Init`
và `Time_Millis`, rồi 2 lỗi `Li005 "no definition"` cho chính 2 hàm đó. Build dừng
ở khâu **Linker**.

## Chuỗi nguyên nhân
1. `main.c` có `#include "time.h"`, nhưng file `time.h` của mình **chưa nằm trong
   include path** của project.
2. IAR tìm file cho `#include "..."` theo thứ tự: thư mục của file nguồn → các đường
   `-I` → **thư mục header chuẩn của trình biên dịch**. Ở đó **có sẵn `<time.h>`
   chuẩn**, nên `"time.h"` **mở nhầm** file chuẩn → **mở được**, do đó KHÔNG phải
   lỗi "cannot open file".
3. `<time.h>` chuẩn không khai báo `Time_Init`/`Time_Millis` → gọi hàm không có
   prototype → trình biên dịch **khai báo ngầm** (Pe223).
4. Vì không có khai báo thật và file định nghĩa (`time.c`) cũng **chưa được add vào
   project**, linker thấy tham chiếu tới `Time_Init`/`Time_Millis` mà không có thân
   hàm → **Li005 no definition**.

## Vị trí code (hiện tại)
`C&C++/Embedded_C99/Microcontroller/stm32f10x/test_lib/stm32f103c8t6/source/main.c` (dòng ~21):
```c
#include "time.h"       // BUG (cũ): đụng <time.h> chuẩn
#include "fsm_time.h"   // FIX hiện tại
```
Module thời gian hiện đặt tại:
`C&C++/Embedded_C99/Microcontroller/stm32f10x/driver/include/fsm_time.h` và
`C&C++/Embedded_C99/Microcontroller/stm32f10x/driver/fsm_time.c`.

## Cách sửa
1. Đổi tên module `time.h`/`time.c` → **`fsm_time.h`/`fsm_time.c`** (tên riêng,
   không thể trùng header chuẩn).
2. Sửa `#include "time.h"` → `#include "fsm_time.h"`.
3. **Add `fsm_time.c` vào project** (Add Files) để linker có thân hàm.

## Bài học / phòng ngừa
- **Đừng đặt tên header cục bộ trùng header chuẩn C** (`time.h`, `string.h`,
  `math.h`, `stdio.h`…). Dùng tiền tố (`fsm_`, tên module…).
- Gặp `Li005 no definition` → kiểm tra file `.c` đã được **add vào project** chưa.
- `Pe223 declared implicitly` (không phải "cannot open file") là dấu hiệu
  `#include "..."` **mở nhầm** một header khác cùng tên.

## Trạng thái
`fixed` — đã đổi sang `fsm_time.*` và add `.c` vào project; build/chạy OK trên board.
