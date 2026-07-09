# KI-GIT-0005 — Tải ZIP repo có submodule → thư mục RỖNG; repo lồng repo → git từ chối track

**Bối cảnh:** nhập kho package chính hãng `STM32CubeF1` vào `Manufacturer_Package/`
của repo này (kế hoạch học hai-track bare-metal ↔ API hãng). Entry này ghi lại
**HAI cái bẫy liên hoàn** khi "đem repo của người khác vào repo của mình" — lần
này được nhận diện TRƯỚC khi dính (status `mitigated`), nhưng nếu dính thì bẫy 1
là build-blocker toàn tập: thiếu sạch HAL/CMSIS, không compile nổi một file.

## Bẫy 1 — nút "Download ZIP" của GitHub bỏ rơi submodule

**Triệu chứng (nếu dính):** giải nén ZIP xong, `Drivers/STM32F1xx_HAL_Driver/`,
`Drivers/CMSIS/Device/ST/STM32F1xx/`… tồn tại nhưng **rỗng tuếch**. Không có
thông báo lỗi nào — thư mục rỗng im lặng, chỉ lộ ra khi build báo thiếu file.

### Chuỗi nguyên nhân
1. Repo `STM32CubeF1` không thật sự CHỨA code driver. Các thư mục đó là
   **submodule** — con trỏ (commit hash + URL) sang repo con riêng biệt
   (`stm32f1xx_hal_driver`, `cmsis_device_f1`, …). Giống tủ hồ sơ chỗ đáng lẽ
   để tài liệu lại dán tờ giấy "hồ sơ gửi ở kho B".
2. Chức năng archive (Download ZIP / tarball) của GitHub chỉ đóng gói **đúng
   repo cha** — theo tài liệu chính thức của GitHub, archive **không bao gồm
   submodule** (xem `refs/`).
3. Đo thực tế trên STM32CubeF1: **22 submodule** (xem `error.log` — HAL driver,
   CMSIS device, 15 BSP, USB host/device, FatFs, FreeRTOS, LwIP) → ZIP thường
   sẽ cho 22 thư mục rỗng.

### Cách né (đã áp dụng)
```bash
git clone --recursive --depth 1 --shallow-submodules \
    https://github.com/STMicroelectronics/STM32CubeF1.git
```
- `--recursive`: đi theo từng "tờ giấy", kéo cả 22 kho con về.
- `--depth 1 --shallow-submodules`: chỉ lấy bản mới nhất, bỏ toàn bộ lịch sử
  hãng (mình xóa `.git` ngay sau đó nên lịch sử là đồ thừa) — tiết kiệm phần
  lớn dung lượng tải.
- Đường thay thế nhẹ hơn: clone thẳng 3 repo con đủ để build
  (`stm32f1xx_hal_driver`, `cmsis_device_f1`, `cmsis_core`), bỏ BSP/middleware.

## Bẫy 2 — repo lồng repo: git mẹ từ chối track file bên trong

**Triệu chứng (nếu dính):** `git add Manufacturer_Package` xong, commit chỉ chứa
MỘT dòng kiểu `160000 commit <hash> STM32CubeF1` (gitlink) thay vì 10.000+ file;
git in cảnh báo `warning: adding embedded git repository`. Người clone repo mẹ
về sẽ nhận thư mục rỗng — bẫy 1 tái sinh ở chính repo của mình.

### Nguyên nhân
Cây clone về mang theo `.git` (repo cha) + metadata submodule. Git gặp thư mục
có `.git` ở BẤT KỲ độ sâu nào là coi đó là repo nhúng và không track nội dung —
lồng sâu thêm mấy lớp thư mục trung gian cũng không thoát.

### Cách né (đã áp dụng)
Xóa sạch metadata git con trước khi `git add`:
```powershell
Get-ChildItem <kho> -Recurse -Force -Filter ".git" |
    ForEach-Object { Remove-Item $_.FullName -Recurse -Force }
Remove-Item <kho>\.gitmodules -Force
```
Verify: đếm lại `.git` còn sót = **0**, rồi mới add. Kết quả thật: commit §11
track đủ **10.653 file** vendor, commit §12 ghi nhận được **53 rename** (`git mv`)
— chính là điều kiện để metric "file hãng nào đã được bài học chạm tới" hoạt động.

## Bài học / phòng ngừa
- Thấy repo GitHub có thư mục kiểu `Drivers/xxx @ abc1234` (dấu `@` + hash trên
  web UI) = submodule → **quên nút Download ZIP đi**, dùng `git clone --recursive`.
- "Đem repo người khác vào repo mình" luôn gồm HAI việc, thiếu một là hỏng:
  (1) lấy đủ file (`--recursive`), (2) xóa `.git` con để repo mẹ quản từng file.
- Thư mục rỗng và gitlink đều **im lặng** — lỗi chỉ phát nổ ở người *clone lại*
  hoặc lúc *build*. Sau khi add kho lớn, luôn đếm số file staged để đối chiếu.

## Trạng thái
`mitigated` — chưa từng dính thật; cả hai bẫy được né chủ động, có số liệu
verify (22 submodule kéo đủ, 0 `.git` sót, 10.653 file tracked, 53 rename).
