# KI-GIT-0003 — URL remote kẹt tên cũ (108) sau khi đổi tên repo GitHub (103)

**Triệu chứng:** `git push origin main` **vẫn thành công**, nhưng in kèm cảnh báo
"repository moved". Repo trên web đã đúng tên **103**, nhưng `git remote -v` dưới máy
vẫn trỏ **108** (cũ). Xem `error.log` cho output nguyên văn.

## Chuỗi nguyên nhân
1. Repo lúc tạo bị gõ nhầm tên `STM32F108C8T6` (108). URL đó ghi cứng vào `.git/config`
   khi `git remote add origin …108….git`.
2. Sau đó đổi tên repo trên **GitHub Settings** → `STM32F103C8T6` (103). **Server = 103.**
3. GitHub, khi một repo đổi tên, **tự lập redirect** từ tên cũ (108) → tên mới (103).
4. **`.git/config` dưới máy KHÔNG được cập nhật** — đổi tên trên server không "với tay"
   xuống máy local. Mỗi lần push, git gửi tới …108…, GitHub redirect sang …103… và vẫn
   nhận → push **luôn OK** → không ai nghi ngờ.
5. Vì push luôn thành công, URL sai nằm im, bị redirect che mất.

**Gốc rễ:** tên repo được lưu ở **HAI nơi độc lập** — (a) server GitHub, (b) `.git/config`
của *mỗi* bản clone — và **chúng không tự đồng bộ với nhau**. `git remote -v` cho biết
"máy đang GỌI tới đâu", **không** phải "server đang TÊN gì".

## Vì sao đáng sửa (không chỉ khó chịu)
- **Rủi ro chiếm tên (chính):** nếu sau này có ai tạo repo mới trùng tên cũ
  `STM32F108C8T6`, tên cũ hết trống → **redirect gãy**, push có thể đi tới repo lạ hoặc
  báo lỗi.
- Mong manh & khó hiểu cho người clone; thiếu chuyên nghiệp với repo để trong CV.

## Vị trí "code" (hiện tại)
Không phải file nguồn — là **cấu hình git** của bản clone:
`.git/config`, mục `[remote "origin"]` → `url = …STM32F108C8T6.git`.
Xem nhanh bằng `git remote -v`.

## Cách sửa (đã áp dụng 2026-07-08)
```bash
git remote set-url origin https://github.com/chinart123/STM32F103C8T6.git
git fetch origin        # xác nhận: hết dòng "This repository moved"
```
(Bản SSH nếu muốn khỏi nhập token: `git@github.com:chinart123/STM32F103C8T6.git`.)

## Bài học / phòng ngừa
- **"Trạng thái server" ≠ "config local".** Đổi tên/đổi chủ repo trên GitHub KHÔNG tự
  sửa `.git/config` của bất kỳ bản clone nào — mỗi người phải tự `git remote set-url`.
- Luôn tin `git remote -v`, đừng tin trí nhớ. Thấy "moved" trong `git fetch/push` nghĩa
  là URL local đã lệch tên server.
- Log push chỉ là output terminal (không có file) — muốn lưu: `git push … 2>&1 | tee push.log`.

## Trạng thái
`fixed` — URL local đã khớp tên server (103), `git fetch` sạch, hết redirect.
