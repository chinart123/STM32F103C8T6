# Hướng dẫn tích hợp — bộ `time` / `button` / `led` (bản scan-process)

> Bộ mới trong `scratch/`, viết theo **đúng pattern ESP32-Quadcopter** của bạn:
> mỗi module lo đúng việc, thời gian tách ra `fsm_time.c`, `main` chỉ chạy logic `if…`.
> Chi tiết từng biến/API xem `THAM-CHIEU-fsm-button-led.md`.

## Kiến trúc
```
fsm_time.h/.c    = đồng hồ ms duy nhất (SysTick + ngắt). Time_Millis() cho ai cần giờ.
fsm_button.h/.c  = KHUNG + Scan (đọc chân, chống dội) + Process (FSM -> Event)
fsm_led.h/.c     = KHUNG + Process (lái chân theo Mode; fade = soft-PWM)
main.c        = chỉ LOGIC: 1 'if' cho mỗi sự kiện nút (không switch/case)
```
- **Nút** không đo thời gian, không đụng LED. **LED** không có Scan (không có input).
- Hiệu ứng **không chặn**, dựa trên `now` (ms) — không `delay()`, không god-`Update()`.
- Biến để **32-bit** hết cho dễ (tối ưu sau).

## Chọn chân — chỉ sửa 2 dòng trong `main.c`
```c
Led_Init(&led, &GPIOC, 13u, LED_ACTIVE_LOW);            // đổi sang chân bất kỳ
Button_Init(&btn, &GPIOA, 0u, BUTTON_ACTIVE_LOW, 800u); // PA0->GND, hold 800 ms
```

## Hành vi mẫu (4 dòng logic trong `main.c`)
| Sự kiện | LED làm gì | API |
|---------|-----------|-----|
| 1 click | bật/tắt | `Led_Set_Toggle` |
| 2 click | **sáng dần** off→full trong 3s, **lặp lại** | `Led_Set_Fade_Up(&led, 3000)` |
| 3 click | **tối dần** full→off trong 3s, **lặp lại** | `Led_Set_Fade_Down(&led, 3000)` |
| giữ ≥800 ms | **nháy nháy** | `Led_Set_Blink(&led, 150, 150)` |

Đổi hành vi → chỉ sửa mấy dòng `if`. `Led_Set_Blink(&led, onMs, offMs)` cho nháy
lệch on/off; đổi `totalMs` trong Fade để nhanh/chậm hơn.

## Vì sao là soft-PWM, không PWM cứng
PC13 (LED onboard) **không có kênh timer** → không thể PWM cứng. Muốn fade (sáng/tối
dần) chạy được **mọi chân**, ta bật/tắt chân rất nhanh trong `Led_Hardware_Process()`
(8 mức × 1 ms ≈ 125 Hz). Không cần file timer, không AF, không NVIC timer.

> **Lưu ý độ mượt:** soft-PWM ở nhịp 1 ms nên chỉ có **8 mức sáng** (giữ 125 Hz không
> nháy). Fade 3 s = 8 nấc, mỗi nấc ~375 ms → nhìn hơi "bậc thang" chứ chưa mượt tuyệt
> đối. Muốn mượt hơn phải hạ nhịp SysTick xuống ~100 µs để tăng số mức — nói mình làm.

## Đưa vào project & build (vòng lặp Option C)
1. Copy 7 file vào project: `fsm_time.h/.c`, `fsm_button.h/.c`, `fsm_led.h/.c`, `main.c`.
   `.h` để vào thư mục include (đang trong include path); `.c` để vào thư mục source.
2. **Add cả 4 file `.c`** (`fsm_time.c`, `fsm_button.c`, `fsm_led.c`, `main.c`) vào
   project để link — thiếu 1 `.c` → lỗi *no definition / undefined symbol*.
   *Bạn tự add trong IAR (Add > Add Files).*
3. **Trùng kiểu:** prefix `fsm_` làm **tên file** hết đụng với `button.h/led.h` ở
   `1_Application`, NHƯNG tên **kiểu** `Button_TypeDef`/`Led_TypeDef` vẫn trùng nếu
   biên dịch cả hai bộ. Chọn 1 bộ. (Đã đổi `time`→`fsm_time` để không đụng
   `<time.h>` chuẩn của thư viện C — chính lỗi Pe223/Li005 gặp phải.)
4. **SysTick_Handler:** `fsm_time.c` đã định nghĩa. Startup của IAR khai báo nó dạng
   *weak* nên tự đè. Đừng để `SysTick_Handler` trùng ở file khác.
5. Build **Make (F7)** cấu hình **Debug**. Lỗi → copy nguyên văn gửi mình.
6. **Download & Debug (Ctrl+D) → Go (F5)**. Kiểm thử 4 hành vi ở bảng trên.

## Ghi chú clock
`Time_Init(8000000u)` giả định lõi **HSI 8 MHz**. Nếu project chạy 72 MHz thì đổi
thành `Time_Init(72000000u)` — mọi thứ khác (ms, fade, hold) tự đúng theo.
