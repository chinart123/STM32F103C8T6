# STM32F103C8T6 — Industrial Library Structure Learning

Dự án nghiên cứu và phân tích chuyên sâu kiến trúc thư viện phần mềm công nghiệp của STMicroelectronics (dành cho dòng vi điều khiển STM32F103C8T6). 

**Mục tiêu cốt lõi:** Phương pháp của dự án không nhằm bài trừ thư viện hãng, mà sử dụng việc lập trình can thiệp trực tiếp thanh ghi (bare-metal C) như một công cụ bóc tách. Quá trình tự xây dựng lại các module giúp thấu hiểu trọn vẹn tư duy thiết kế, cách tổ chức mã nguồn và kiến trúc nền tảng của thư viện HAL/LL chuẩn.

**Hướng dẫn tiếp cận dự án:**
*   **Bước 1:** Đọc file `AGENTS.md` để nắm rõ các quy tắc đối chiếu và tiêu chuẩn xây dựng mã nguồn.
*   **Bước 2:** Tham khảo `docs/platform-v2.md` để hiểu kiến trúc tổng thể (platform blueprint) đang được mô phỏng lại.
*   **Bước 3:** Truy cập `docs/detected-issues/` để tra cứu các lỗi kỹ thuật phát sinh trong quá trình bóc tách phần cứng và cách khắc phục.

---

## 📦 Chi tiết các thành phần dự án

Dự án triển khai mô hình học tập "song song" (Dual-track), chia mã nguồn thành các khu vực đối chiếu rõ ràng:

*   **`C&C++/`**
    *   Thư mục chứa mã nguồn firmware tự phát triển 100% ở mức thanh ghi (bare-metal).
    *   Tuân thủ nghiêm ngặt quy tắc NO HAL / NO LL trong nhân firmware chính để phục vụ việc học cốt lõi.

*   **`Take note quá trình học thanh ghi/`**
    *   Tài liệu ghi chép cá nhân trong từng buổi học thực hành phân tích thư viện (ngôn ngữ Tiếng Việt).

*   **`docs/`**
    *   Lưu trữ các bản thiết kế kiến trúc (blueprint) và kho lỗi (detected-issues).
    *   Được tổ chức có cấu trúc nhằm phục vụ quá trình tra cứu của kỹ sư và các tác vụ phân tích của AI.

*   **`Manufacturer_Package/` — Dữ liệu đối chiếu gốc (Provenance)**
    *   Khu vực này tuyệt đối không chứa mã nguồn tự viết, chỉ lưu trữ mã nguồn nguyên bản từ STMicroelectronics để làm "kim chỉ nam" tham chiếu kiến trúc. Bao gồm:
    *   `STM32CubeF1/`: Bản sao (clone) nguyên trạng từ repository chính thức của hãng (kèm 3 submodule driver). Mã nguồn tuân thủ giấy phép **BSD-3-Clause** và file `LICENSE.md` gốc được giữ nguyên. Các metadata của `.git` đã được loại bỏ để dự án tự quản lý độc lập.
    *   `No.0_C&C++_Industrial_Draft/`: Đây chính là kết quả của quá trình tự "clone" lại các driver dựa trên việc phân tích cấu trúc từ thư viện hãng. Chứa các file gốc của ST (giữ nguyên 100% tên và nội dung) nhưng được **di chuyển** vào mô hình thư mục phân cấp giống hệt với `C&C++/`. 
        *   *Mục đích:* Học tập song song — viết bản bare-metal tự tay bên `C&C++/` trước, sau đó đối chiếu trực tiếp với cách tổ chức API của hãng bên này. 
        *   *Lập bản đồ tiến độ:* File hãng chỉ được "nhập kho" vào thư mục Draft này khi lộ trình học chạm đến. Lịch sử commit (git history) tại đây phản ánh chính xác tiến trình tiếp thu tư duy thiết kế của hãng. Mỗi lần tái cấu trúc lớn sẽ tạo đợt mới (`No.1_`, `No.2_`...).

---

## 🗂️ Cấu trúc cây thư mục tổng quan

```text
.
├── AGENTS.md                               # Quy tắc và tiêu chuẩn dự án
├── C&C++/                                  # Firmware tự phát triển (100% bare-metal)
├── docs/                                   # Tài liệu kỹ thuật, kiến trúc và kho lỗi
│   ├── detected-issues/                    # Kho lưu trữ các lỗi đã gặp
│   └── platform-v2.md                      # Blueprint kiến trúc platform
├── Manufacturer_Package/                   # Dữ liệu đối chiếu gốc (Provenance)
│   ├── No.0_C&C++_Industrial_Draft/        # Bản tái cấu trúc driver từ thư viện hãng
│   └── STM32CubeF1/                        # Bản clone nguyên trạng từ ST
└── Take note quá trình học thanh ghi/      # Ghi chú học tập cá nhân
```
