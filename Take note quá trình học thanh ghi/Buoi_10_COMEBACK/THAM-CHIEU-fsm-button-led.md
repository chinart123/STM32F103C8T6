# Tham chiếu chi tiết — `time` · `button` · `led` (bản scan/process)

> Giải thích **từng biến** trong struct và **từng API** của bộ driver mới. Kiến trúc
> theo đúng repo **ESP32-Quadcopter** của bạn: mỗi module lo đúng việc của nó,
> thời gian tách hẳn ra `fsm_time.c`, `main` chỉ chạy logic bằng `if…`.
>
> Quy ước: `now` = mili-giây lấy từ `Time_Millis()`. Mọi biến để **32-bit** (`unsigned int`).

---

## Luồng chạy tổng thể (mỗi 1 ms trong `main`)
```
now = Time_Millis();                 // fsm_time.c: đồng hồ ms duy nhất của hệ
Button_Hardware_Scan(&btn);          // đọc chân + chống dội  -> btn.Pressed
Button_Hardware_Process(&btn, now);  // FSM click/hold        -> btn.Event
if (btn.Event == ...) Led_Set_*();   // LOGIC: 1 if / 1 sự kiện (không switch)
Led_Hardware_Process(&led, now);     // xuất chân LED theo Mode (soft-PWM nếu fade)
```
`button` không đo thời gian, không đụng LED. `led` không có input nên **không có Scan**,
chỉ có `Process` để lái chân. `time` là nơi **duy nhất** sở hữu thời gian.

---

# PHẦN A — MODULE `fsm_time` (SysTick)

## A1. API (`fsm_time.h`)
| API | Tham số | Nhiệm vụ |
|-----|---------|----------|
| `Time_Init(coreHz)` | tần số lõi (Hz) | Nạp `STK.LOAD = coreHz/1000 - 1` (1 ms), bật SysTick **kèm ngắt** (`CTRL=7`). Gọi 1 lần đầu. |
| `Time_Millis()` | — | Trả số **ms** đã trôi từ lúc `Time_Init` (tràn sau ~49 ngày). Mọi module hỏi giờ qua đây. |
| `Time_Delay(ms)` | ms | Chờ bận `ms` mili-giây. **Chỉ dùng lúc setup**, không dùng trong vòng lặp chính. |

## A2. Biến & hàm nội bộ (`fsm_time.c`)
| Tên | Ý nghĩa |
|-----|---------|
| `g_ms` (`static volatile unsigned int`) | Bộ đếm ms toàn cục. `Time_Millis` đọc nó. |
| `SysTick_Handler()` | Ngắt SysTick, chạy **mỗi 1 ms**, chỉ làm `g_ms++`. IAR startup đã khai báo weak nên hàm này tự đè lên. **Đừng** định nghĩa `SysTick_Handler` ở nơi khác. |

> SysTick là **exception hệ thống** (không phải IRQ ngoài) → chỉ cần bit `TICKINT`,
> **không cần cấu hình NVIC**. Rất gọn.

---

# PHẦN B — MODULE `button`

## B1. Hằng số & enum (`fsm_button.h`)
| Tên | Giá trị | Ý nghĩa |
|-----|---------|---------|
| `BTN_DEBOUNCE_MS` | `15u` | Nhấn ngắn hơn mức này bị bỏ (nhiễu). |
| `BTN_CLICK_GAP_MS` | `300u` | Im lặng đủ lâu này thì **chốt** chuỗi click. |
| `BUTTON_ACTIVE_LOW/HIGH` | 1 / 0 | Cực tính nút. |
| `BTN_EVENT_NONE/SINGLE/DOUBLE/TRIPLE/HOLD` | 0..4 | Sự kiện FSM phát ra. |

## B2. Biến trong `Button_TypeDef`
### Binding — gắn phần cứng (đặt trong `Button_Init`)
| Biến | Ý nghĩa |
|------|---------|
| `Port` | Con trỏ port (`&GPIOA`…) → pin-agnostic. |
| `Pin` | Số chân 0..15. |
| `ActiveLow` | 1 = active-low. |
| `HoldTargetMs` | Giữ bao nhiêu ms thì phát `HOLD`. |

### Do `Hardware_Scan` điền
| Biến | Ý nghĩa |
|------|---------|
| `History` | Thanh ghi dịch 8 bit chứa 8 mẫu đọc gần nhất (chống dội). |
| `Pressed` | Trạng thái **đã lọc dội**: 1 khi 8 mẫu liên tục = nhấn. |

### FSM dùng trong `Hardware_Process`
| Biến | Ý nghĩa |
|------|---------|
| `WasPressed` | `Pressed` của tick trước → phát hiện cạnh nhấn/nhả. |
| `PressStartMs` | Mốc ms khi bắt đầu nhấn (đo độ dài nhấn & hold). |
| `LastReleaseMs` | Mốc ms lần nhả gần nhất (đo khoảng lặng kết chuỗi). |
| `ClickCount` | Số click đã đếm trong chuỗi hiện tại. |
| `HoldFired` | Cờ chống lặp: `HOLD` chỉ phát 1 lần / lần nhấn; và hold **không** bị tính thành click. |

### Đầu ra
| Biến | Ý nghĩa |
|------|---------|
| `Event` | Kết quả tick này (`NONE`/`SINGLE`/…). `main` đọc ngay sau `Process`. Bị reset về `NONE` mỗi lần gọi `Process`. |

## B3. Hàm nội bộ (`static`, `fsm_button.c`)
| Hàm | Làm gì |
|-----|--------|
| `btn_clock(port)` | Bật clock APB2 đúng port (so con trỏ A..E). |
| `btn_input_pull(port,pin,pullUp)` | Ghi nibble `0x8` (input+pull) vào CRL/CRH; ODR chọn kéo lên/xuống. |
| `btn_raw_pressed(btn)` | Đọc `IDR`, đổi theo cực tính → "đang nhấn?" thô. |

## B4. API công khai (`fsm_button.h`)
| API | Nhiệm vụ |
|-----|----------|
| `Button_Init(&btn, port, pin, activeLow, holdMs)` | Gắn chân input+pull đúng cực tính, đặt ngưỡng hold, xoá state. |
| `Button_Hardware_Scan(&btn)` | **Chỉ đọc + chống dội** → `Pressed`. Không đụng thời gian/FSM. Gọi mỗi 1 ms. |
| `Button_Hardware_Process(&btn, now)` | **Chỉ FSM** dùng `Pressed` + `now` → `Event` (single/double/triple/hold). |

> **Vòng đời chuỗi click:** cạnh nhả hợp lệ → `ClickCount++`, ghi `LastReleaseMs`.
> Nếu im lặng ≥ `BTN_CLICK_GAP_MS` → phát `SINGLE/DOUBLE/TRIPLE` theo `ClickCount`
> rồi reset. Nếu nhấn giữ tới `HoldTargetMs` → `HOLD` (và huỷ đếm click).

---

# PHẦN C — MODULE `led`

## C1. Hằng số & enum (`fsm_led.h`)
| Tên | Ý nghĩa |
|-----|---------|
| `LED_PWM_STEPS` = `8u` | Số mức sáng của soft-PWM (0..8). 8 mức × 1 ms ≈ 125 Hz, không nháy. |
| `LED_ACTIVE_LOW/HIGH` | Cực tính LED. |
| `LED_MODE_OFF/ON/BLINK/FADE_UP/FADE_DOWN` | Chế độ hiện tại của LED. |

## C2. Biến trong `Led_TypeDef`
### Binding
| Biến | Ý nghĩa |
|------|---------|
| `Port` / `Pin` / `ActiveLow` | Gắn chân + cực tính (giống button). |

### Chế độ
| Biến | Ý nghĩa |
|------|---------|
| `Mode` | Một trong `Led_Mode`. `Process` rẽ nhánh theo biến này. |

### Blink (nháy lệch on/off)
| Biến | Ý nghĩa |
|------|---------|
| `OnMs` / `OffMs` | Độ dài pha bật / tắt (ms), khác nhau ⇒ nháy lệch. |
| `PhaseStartMs` | Mốc ms pha hiện tại bắt đầu. |
| `PhaseOn` | 1 = đang ở pha bật. |

### Fade (soft-PWM)
| Biến | Ý nghĩa |
|------|---------|
| `Duty` | Độ sáng hiện tại 0..`LED_PWM_STEPS` (số ms "bật" trong mỗi cửa sổ 8 ms). |
| `StepMs` | Bao nhiêu ms thì đổi độ sáng 1 mức = `totalMs / LED_PWM_STEPS`. |
| `StepStartMs` | Mốc ms lần đổi mức gần nhất. |

## C3. Hàm nội bộ (`static`, `fsm_led.c`)
| Hàm | Làm gì |
|-----|--------|
| `led_clock(port)` | Bật clock APB2 đúng port. |
| `led_out_2mhz(port,pin)` | Ghi nibble `0x2` (output push-pull 2 MHz) vào CRL/CRH. |
| `led_pin(led,lit)` | Đổi cờ "sáng?" → mức chân thật (áp cực tính), ghi BSRR/BRR. |
| `led_pwm(led,now)` | Soft-PWM: bật chân khi `(now % 8) < Duty`. Đây là chỗ tạo độ sáng. |

## C4. API công khai (`fsm_led.h`)
| API | Nhiệm vụ |
|-----|----------|
| `Led_Init(&led, port, pin, activeLow)` | Gắn chân, đặt output 2 MHz, tắt LED. |
| `Led_Set_On/Off(&led)` | Đặt `Mode = ON/OFF`. |
| `Led_Set_Toggle(&led)` | Đảo giữa ON↔OFF. |
| `Led_Set_Blink(&led, onMs, offMs)` | `Mode = BLINK`, nháy lệch on/off. |
| `Led_Set_Fade_Up(&led, totalMs)` | `Mode = FADE_UP`: "sáng dần" off→full trong `totalMs` rồi **lặp lại** (soft-PWM). |
| `Led_Set_Fade_Down(&led, totalMs)` | `Mode = FADE_DOWN`: "tối dần" full→off trong `totalMs` rồi **lặp lại** (soft-PWM). |
| `Led_Hardware_Process(&led, now)` | Lái chân theo `Mode` (mỗi `if` = 1 chế độ, không switch). Gọi mỗi 1 ms. |

> **Vì sao soft-PWM chứ không PWM cứng?** PWM cứng chỉ ra ở chân có kênh timer;
> **PC13 không có** → không thể PWM cứng. Soft-PWM (bật/tắt nhanh trong `Process`)
> chạy được **mọi chân** mà không cần file timer nào.

---

# PHẦN D — `main.c` (chỉ logic)
```c
if (btn.Event == BTN_EVENT_SINGLE) Led_Set_Toggle(&led);   // 1 click: bật/tắt
if (btn.Event == BTN_EVENT_DOUBLE) Led_Set_Fade_Up(&led, 3000);   // 2 click: sáng dần 3s, lặp
if (btn.Event == BTN_EVENT_TRIPLE) Led_Set_Fade_Down(&led, 3000); // 3 click: tối dần 3s, lặp
if (btn.Event == BTN_EVENT_HOLD)   Led_Set_Blink(&led, 150, 150); // giữ: nháy nháy
```
Đổi chân → sửa 2 dòng `*_Init`. Đổi hành vi → sửa 4 dòng `if`. Driver giữ nguyên.

---

# PHẦN E — Bảng đổi tên (điền nếu vẫn chưa ưng)
| Nhóm | Tên hiện tại | Tên mới (bạn điền) |
|------|--------------|--------------------|
| time | `Time_Init` / `Time_Millis` / `Time_Delay` | |
| button | `Button_Hardware_Scan` / `Button_Hardware_Process` | |
| button | `Button_Event` + `BTN_EVENT_*` | |
| led | `Led_Hardware_Process` | |
| led | `Led_Set_On/Off/Toggle/Blink/Fade_Up/Fade_Down` | |
| led | `Led_Mode` + `LED_MODE_*` | |
| kiểu | `Button_TypeDef` / `Led_TypeDef` | |
