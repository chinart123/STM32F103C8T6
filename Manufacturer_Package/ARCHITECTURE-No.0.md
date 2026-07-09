# ARCHITECTURE — Manufacturer_Package / No.0_C&C++_Industrial_Draft
(raw text, 2026-07-09 — file kiểm tra chéo cho user, trả lời 3 câu hỏi + recheck lỗi IAR tiềm tàng)

## 0. CÂY THƯ MỤC THỰC TẾ (sau §11 + §12)

D:\libraries\
├── C&C++\                                  <- firmware TỰ VIẾT, NO HAL/NO LL (không đụng)
│   └── Embedded_C99\Microcontroller\
│       ├── 0_common\           (common.c + include\common.h, define.h)
│       ├── 1_Application\      (button, led, fsm_*)
│       └── stm32f10x\
│           ├── driver\         (gpio.c, fsm_time.c + include\*.h tự viết)
│           └── test_lib\stm32f103c8t6\  (test.eww/.ewp/.ewd - project IAR DUY NHẤT)
│
└── Manufacturer_Package\
    ├── SIZE-CHECK.md                       <- audit dung lượng (max 13.06MB < 100MB)
    ├── ARCHITECTURE-No.0.md                <- file này
    ├── _commitmsg_12.txt                   <- message §-commit mới nhất (luật B1)
    │
    ├── STM32CubeF1\                        <- KHO GỐC của hãng (ĐÃ BỊ RÚT 53 file)
    │   ├── LICENSE.md                      <- BSD-3-Clause, giữ nguyên chỗ
    │   ├── Drivers\
    │   │   ├── STM32F1xx_HAL_Driver\
    │   │   │   ├── Src\                    <- CÒN 35 .c (adc, can, cortex, DMA, dac, usb…)
    │   │   │   └── Inc\                    <- CÒN 40 .h + Legacy\stm32_hal_legacy.h
    │   │   │                                  + stm32f1xx_hal_conf_template.h  (QUAN TRỌNG, xem mục 2)
    │   │   └── CMSIS\
    │   │       ├── Include\                <- core_cm3.h … (tầng lõi Cortex-M)
    │   │       └── Device\ST\STM32F1xx\
    │   │           ├── Include\            <- stm32f1xx.h, stm32f103xb.h, system_stm32f1xx.h
    │   │           └── Source\Templates\iar\ <- startup_stm32f103xb.s (khi cần)
    │   ├── Middlewares\  Projects\  Utilities\  Documentation\   <- chưa đụng, nằm chờ
    │   └── (KHÔNG còn bất kỳ .git / .gitmodules nào — đã xóa sạch, verify = 0)
    │
    └── No.0_C&C++_Industrial_Draft\        <- KHO ĐÃ SẮP XẾP theo cấu trúc C&C++\
        └── Embedded_C99\Microcontroller\      (mirror 1:1 vị trí, KHÔNG có project IAR)
            ├── 0_common\
            │   ├── stm32f1xx_hal.c                 <- vai của common.c
            │   └── include\stm32f1xx_hal_def.h     <- vai của common.h
            └── stm32f10x\driver\
                ├── stm32f1xx_hal_gpio.c  _tim.c  _rcc.c  _exti.c  _flash.c
                │   _i2c.c  _pwr.c  _rtc.c  _spi.c  _uart.c  _usart.c
                │   (+ bản _ex.c và ll_*.c) — TỔNG 25 file .c
                └── include\  (26 file .h tương ứng + ll_bus.h)

## 1. ĐÃ GIẢI QUYẾT "REPO LỒNG REPO" CHƯA? — RỒI. Minh chứng:

- Trong cây trên KHÔNG tồn tại `.git`/`.gitmodules` nào dưới Manufacturer_Package\
  (lệnh đếm sau khi xóa đệ quy trả về 0 — ghi trong KI-GIT-0005/error.log).
- Bằng chứng git ĐANG track từng file (điều mà embedded repo không cho phép):
  + commit §11 (6482b52) chứa 10.653 file vendor ADD từng file một
    (nếu còn lồng repo, commit chỉ chứa 1 dòng gitlink `160000 <hash>`)
  + commit §12 (184824b) ghi nhận 53 RENAME — git chỉ rename được file nó quản
- Kết luận: người clone repo này về là có ĐỦ ruột kho hãng, không dính bẫy
  submodule-rỗng ngược (KI-GIT-0005 bẫy 2 đã bị vô hiệu).

## 2. CÓ TIỆN COMPILE / BUILD / DOWNLOAD-DEBUG KHÔNG? — CÓ, VỚI CÔNG THỨC SAU

Kiến trúc này tách "KHO" (file .c/.h nằm yên) khỏi "PROJECT" (test.ewp quyết
định compile cái gì). IAR không tự quét thư mục — file trong kho KHÔNG tự chui
vào build, nên thêm kho không thể phá build hiện tại. Muốn DÙNG API hãng, làm
đúng 3 việc trong project:

  (a) Include path — thêm vào tab C/C++ Compiler > Preprocessor (5 đường, đều $PROJ_DIR$-relative):
      1. ...\Manufacturer_Package\No.0_C&C++_Industrial_Draft\Embedded_C99\Microcontroller\stm32f10x\driver\include
      2. ...\No.0_C&C++_Industrial_Draft\Embedded_C99\Microcontroller\0_common\include
      3. ...\Manufacturer_Package\STM32CubeF1\Drivers\STM32F1xx_HAL_Driver\Inc     <- vì Legacy\ + hal_conf
      4. ...\Manufacturer_Package\STM32CubeF1\Drivers\CMSIS\Device\ST\STM32F1xx\Include
      5. ...\Manufacturer_Package\STM32CubeF1\Drivers\CMSIS\Include
      MINH CHỨNG cần đủ 5 đường (đo thật trên file đã nhập kho):
      stm32f1xx_hal_def.h dòng 29-30:  #include "stm32f1xx.h"            -> cần (4)
                                       #include "Legacy/stm32_hal_legacy.h" -> cần (3)
      stm32f1xx.h -> #include core_cm3/system_stm32f1xx                  -> cần (5)(4)
  (b) Defined symbols: STM32F103xB  (+ USE_FULL_LL_DRIVER khi dùng LL)
  (c) Đăng ký file .c cần thiết vào project (Add Files) — include path KHÔNG
      thay được việc này; thiếu là "Error[Li005]: undefined external" ngay.
      Bộ tối thiểu cho HAL-GPIO demo: stm32f1xx_hal.c + hal_cortex.c(kho)
      + hal_rcc.c + hal_gpio.c. Dùng LL thuần thì thường chỉ cần header (đa số
      LL là static inline) — nhẹ hơn hẳn.

Lỗi lặt vặt ĐÃ RECHECK — 7 bẫy tiềm tàng và trạng thái:
  1. thiếu stm32f1xx_hal_conf.h  -> HAL .c nào cũng đòi nó; hãng chỉ giao
     *_conf_template.h. PHẢI copy template -> đổi tên -> bật HAL_MODULE_ENABLED
     cần dùng. Đây là file CONFIG CỦA MÌNH (sẽ tạo ở bài học đầu tiên dùng HAL,
     đề xuất đặt cạnh main của bài). TRẠNG THÁI: chưa tạo — việc của Buổi 12 track 2.
  2. đường dẫn chứa "&" (C&C++)  -> hand-edit .ewp phải escape thành &amp;
     (XML); thêm bằng GUI của IAR thì GUI tự lo. TRẠNG THÁI: lưu ý khi sửa tay.
  3. header trùng tên đè nhau (họ KI-IAR-0001 time.h) -> tên hãng đều có tiền
     tố stm32f1xx_/core_ -> KHÔNG đụng header chuẩn C, KHÔNG đụng gpio.h/rcc.h
     tự viết. TRẠNG THÁI: an toàn theo thiết kế.
  4. trùng symbol khi link (2 định nghĩa cùng hàm) -> chỉ xảy ra nếu đăng ký
     cả driver tự viết LẪN file hãng cùng chức năng vào CÙNG build config.
     Driver 2 bên khác tên hàm (GPIO_Init vs HAL_GPIO_Init) nên mặc định KHÔNG
     trùng; các file *_template.c của hãng (msp/timebase) tuyệt đối không add.
     TRẠNG THÁI: an toàn nếu theo công thức (c).
  5. HAL timeout treo (I2C/UART) -> HAL_Delay/HAL_GetTick cần HAL_Init() +
     SysTick chạy; code bare-metal không gọi HAL_Init. TRẠNG THÁI: ghi nhớ cho
     bài track-2 đầu tiên — demo HAL phải gọi HAL_Init() trước.
  6. Download & Debug -> KHÔNG ảnh hưởng: test.ewd/ST-Link/flash loader không
     đổi; thêm code HAL chỉ tăng size flash (F103C8 = 64KB, bài GPIO/DMA dư sức).
     TRẠNG THÁI: an toàn.
  7. build config -> nên tạo config IAR RIÊNG (vd "Debug_Vendor") trong test.ewp
     thay vì nhét include path vendor vào config Debug thường — giữ ranh giới
     NO HAL/NO LL cho build mặc định; bấm chuyển config là đổi track.
     TRẠNG THÁI: đề xuất, chốt ở Buổi 12.

## 3. TRẢ LỜI NHANH: "RÚT RUỘT" HAY "COPY"?

ĐÃ RÚT RUỘT (git mv = DI CHUYỂN, không copy):
- STM32CubeF1\ KHÔNG còn nguyên vẹn như hãng giao: Src còn 35/60 file .c,
  Inc còn 40/67 file .h — 53 file đã CHUYỂN HẲN sang No.0_..._Draft\.
- No.0_..._Draft\ + STM32CubeF1\ GỘP LẠI = đúng 100% bộ hãng (không mất file
  nào, không file nào tồn tại 2 nơi).
- Muốn xem bản pristine của hãng: GitHub STMicroelectronics/STM32CubeF1
  (hoặc re-clone; kỷ nguyên No.1 sau này cũng sẽ clone tươi lại từ đầu).

VÌ SAO move mà không copy (quyết định có chủ đích):
- git history của file Draft = ngày bài học chạm tới nó (`git log --follow`)
  -> metric % kiến thức cho biểu đồ mỗi kỷ nguyên No.xx. Copy thì mọi file
  xuất hiện ngày đầu, metric chết.
- Copy tạo 2 bản cùng tên trong 2 include path -> compiler lấy bản nào tùy thứ
  tự path = bug tiềm ẩn đúng họ KI-IAR-0001. Move thì mỗi header chỉ có 1 nhà.
