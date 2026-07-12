# Playbook khôi phục kho hãng STM32CubeF1

> Kích hoạt khi: quá trình rút ruột làm hỏng file trong `No.0_C&C++_Industrial_Draft/` hoặc `STM32CubeF1/` đến mức khó cứu (mất đoạn code không nhớ đã comment gì, xóa nhầm, merge nát…).

## Bối cảnh phải nhớ trước khi làm

- `Manufacturer_Package/STM32CubeF1/` là bản clone từ `https://github.com/STMicroelectronics/STM32CubeF1` nhưng **đã gỡ `.git` riêng** — mọi file của nó do repo ngoài (`D:\libraries`) quản. Không có git-lồng-git ở trạng thái bình thường.
- Commit **§12** đã `git mv` (không copy) 53 file từ `STM32CubeF1/` sang `No.0_C&C++_Industrial_Draft/` → kho gốc hiện **không còn** các file đó; bản nguyên thủy nằm trong git history (import ở **§11**).

## Bước 0 — LUÔN thử git history trước (rẻ nhất, nhanh nhất)

Hầu hết tai nạn không cần clone lại. Bản gốc của mọi file hãng còn nguyên trong lịch sử:

```bash
# file bị hỏng vào repo ở commit nào?
git log --oneline --diff-filter=A -- "<đường/dẫn/file>"

# xem bản nguyên thủy (thay <commit> bằng hash tìm được, thường là commit §11):
git show "<commit>:<đường/dẫn/file>"

# khôi phục nguyên trạng file về working tree:
git checkout "<commit>" -- "<đường/dẫn/file>"
```

Chỉ khi git history cũng không cứu được (hỏng trên diện quá rộng, hoặc cần file chưa từng vào repo) mới đi tiếp Bước 1.

## Bước 1 — Clone lại vào chỗ TẠM, không clone đè

```bash
git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeF1.git /tmp/cube_fresh
```

Lưu ý chọn đúng tag/release trùng bản đã dùng nếu cần đối chiếu từng dòng (xem tag trong `git log` của bản cũ hoặc `package.xml` của kho hiện tại).

## Bước 2 — Gỡ git-lồng-git NGAY

```bash
rm -rf /tmp/cube_fresh/.git
```

Bắt buộc làm trước khi đưa vào cây repo. Nếu quên: repo ngoài sẽ coi thư mục này là "embedded repository" (gitlink) — `git add` chỉ ghi một con trỏ SHA thay vì track file, và toàn bộ nội dung kho hãng biến mất khỏi khả năng quản lý của repo ngoài.

## Bước 3 — Đặt vào đúng vị trí

Tùy mức hỏng:

- **Hỏng vài file:** copy đúng các file đó từ `/tmp/cube_fresh/` sang vị trí tương ứng (trong `STM32CubeF1/` hoặc `No.0_Draft/` tùy file thuộc vùng nào). Nhớ: file trong `No.0_Draft/` là bản RÚT RUỘT — sau khi chép bản nguyên thủy về phải rút ruột lại (comment, không xóa) theo đúng những gì các build đang cần.
- **Hỏng cả kho:** thay nguyên thư mục `Manufacturer_Package/STM32CubeF1/` bằng `/tmp/cube_fresh/`. Sau đó tái hiện §12: `git mv` lại đúng 53 file sang `No.0_Draft/` (danh sách lấy từ `git show §12 --stat`).

## Bước 4 — Đối chiếu số lượng file và API đã dùng

```bash
# 1) đếm file hai bên (kho mới vs trạng thái repo trước tai nạn):
find Manufacturer_Package/STM32CubeF1 -type f | wc -l
git ls-tree -r HEAD --name-only -- Manufacturer_Package/STM32CubeF1 | wc -l

# 2) git status/diff cho thấy chính xác cái gì đổi so với HEAD:
git status --short -- Manufacturer_Package/
git diff --stat HEAD -- Manufacturer_Package/

# 3) kiểm tra API đang dùng còn nguyên: build lại các config ăn file hãng —
#    linker chính là máy so API (thiếu hàm nào nó đòi nợ hàm đó, xem Li005):
"C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.3\common\bin\IarBuild.exe" \
    "C&C++/Embedded_C99/Microcontroller/stm32f10x/test_lib/stm32f103c8t6/test.ewp" -build DMA_B12_S01_HAL -log warnings
# ... và Debug_Vendor. Chuẩn PASS: Total number of errors: 0
```

## Bước 5 — Chốt sổ

Diff sẽ rất lớn (cả kho hãng) — **không tự commit**; báo cáo cho user số liệu đối chiếu (file count, danh sách lệch, kết quả build) và chờ lệnh. Ghi bug_log về tai nạn + nguyên nhân để lần sau không tái diễn.
