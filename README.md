# STM32F103C8T6 — bare-metal register learning project

Dự án tự học lập trình thanh ghi STM32F103C8T6 (Blue Pill), **bare-metal C,
không HAL / không LL** trong firmware chính. Bắt đầu đọc từ `AGENTS.md` (luật
dự án) → `docs/platform-v2.md` (blueprint platform) → `docs/detected-issues/`
(kho lỗi đã gặp).

- `C&C++/` — firmware tự viết, 100% register-level (luật NO HAL/NO LL).
- `Take note quá trình học thanh ghi/` — note tay từng buổi học (tiếng Việt).
- `docs/` — blueprint + kho detected-issues cho người và AI đọc.

## Manufacturer_Package/ — xuất xứ (provenance)

Thư mục này chứa **code chính hãng STMicroelectronics**, công khai minh bạch,
không phải code tự viết:

- `STM32CubeF1/` — clone nguyên trạng từ
  <https://github.com/STMicroelectronics/STM32CubeF1> (kèm 3 submodule driver),
  license **BSD-3-Clause** — file `LICENSE.md` của ST giữ nguyên bên trong.
  Đã xóa metadata `.git` để git của repo này quản từng file.
- `No.0_C&C++_Industrial_Draft/` — CÙNG các file hãng đó (tên + nội dung giữ
  nguyên 100%), chỉ **di chuyển** vào cấu trúc thư mục giống hệt `C&C++/`.
  Mục đích học tập hai-track: mỗi bài học viết bản bare-metal tự tay trước,
  rồi đối chiếu bản dùng API hãng — file hãng chỉ được "nhập kho" Draft khi
  bài học chạm tới nó, nên git history của thư mục này chính là bản đồ
  tiến độ kiến thức. Mỗi lần refactor lớn sẽ tạo đợt mới `No.1_`, `No.2_`…
- `SIZE-CHECK.md` — bảng kiểm tra chéo dung lượng (giới hạn GitHub 100MB/file).
