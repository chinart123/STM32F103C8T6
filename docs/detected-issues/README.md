# Kho lỗi đã phát hiện (Detected-Issues)

Nơi ghi lại các lỗi **KHÔNG phải logic firmware của mình** — mà đến từ toolchain,
ngôn ngữ C, filesystem, phần cứng/MCU, hay khác biệt máy/môi trường. Đây là
**kiến thức tái dùng** cho mọi project cùng công cụ, nên được **version + push lên
remote**.

> Lỗi **logic firmware** ("bug tự hành") ở chỗ khác: `Log-and-Report-writing-tools/logs/bug_log.md`
> (local, git-ignore). Kho này chỉ chứa lỗi hệ thống/công cụ/phần cứng.

## Phân loại theo NGUỒN GỐC (`origin`, danh sách MỞ)
Các nhóm origin nằm gọn dưới thư mục trung gian **`categories-by-origin/`**:

| origin | Nghĩa | Thư mục (trong `categories-by-origin/`) |
|--------|-------|---------|
| `toolchain` | IAR EWARM: compiler / linker / IDE | `toolchain-iar/` |
| `language` | C / preprocessor / thư viện chuẩn | `language-c/` |
| `filesystem` | hoa/thường tên file, UTF-8, đường dẫn | `filesystem-encoding/` |
| `hardware` | MCU/silicon: cùng code chip này chạy chip kia không; errata | `hardware-mcu/` |
| `host-environment` | khác máy/OS, khác phiên bản tool, artifact/cache bẩn | `host-environment/` |

Danh sách này **mở** — gặp loại mới thì thêm thư mục + hàng ở đây. Một lỗi có thể
mang **nhiều `origin`** (đa tag trong `meta.yml`).

## Mã định danh
`KI-<CÔNG_CỤ|MIỀN>-<NNNN>` + slug mô tả. Đánh số **tăng dần toàn kho**.
Ví dụ: `KI-IAR-0001`, `KI-ENV-0002`, `KI-HW-0003`.

## Mỗi lỗi = 1 thư mục, nhiều loại file
| File | Vai trò |
|------|---------|
| `README.md` | Phân tích: triệu chứng → chuỗi nguyên nhân → **snippet + vị trí hiện tại** → fix → bài học |
| `meta.yml` | Máy đọc: id, mã lỗi, tool+phiên bản, origin, severity, **mốc thời gian có giờ**, tag |
| `error.log` | Output **NGUYÊN VĂN** của công cụ (không sửa) |
| `refs/<tên-tài-liệu>.md` | Mỗi tài liệu hãng 1 file, **đặt tên theo tài liệu**: định danh + trích đoạn + liên hệ bug + snippet/vị trí |

Copy `_TEMPLATE/` (ở gốc kho) để tạo entry mới, đặt vào
`categories-by-origin/<nhóm-origin>/KI-<...>__slug/`.

## Quy ước
- **Song ngữ:** phần phân tích = tiếng Việt; `error.log` + trích đoạn hãng = **nguyên văn**.
- **UTF-8** toàn bộ.
- Mốc thời gian trong `meta.yml` ghi **có giờ + múi giờ** (vd `2026-07-07 20:05 (+07:00)`).

## Danh mục lỗi
| ID | Tiêu đề | origin | status |
|----|---------|--------|--------|
| [KI-IAR-0001](categories-by-origin/toolchain-iar/KI-IAR-0001__time-h-shadows-standard-header/) | `time.h` cục bộ bị `<time.h>` chuẩn che mất | toolchain, language | fixed |
| [KI-ENV-0002](categories-by-origin/host-environment/KI-ENV-0002__rcc-init-not-found-identical-source/) | Cùng source, `RCC_Init` không tìm thấy trên 1 máy | host-environment, toolchain | open |
