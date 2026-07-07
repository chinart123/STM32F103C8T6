# Git Documentation — git-remote

## 1. Định danh tài liệu
- **Tên đầy đủ:** git-remote — Manage set of tracked repositories
- **Nhà phát hành:** Git (Software Freedom Conservancy) — git-scm.com
- **Phiên bản:** git ≥ 2.x (lệnh `set-url` có từ lâu)
- **Ngày phát hành / sửa đổi:** theo bản git đang cài (ghi bản cụ thể nếu cần đối chiếu)
- **Link:** https://git-scm.com/docs/git-remote
- **Mục/section liên quan:** `git remote set-url`
- **Cú pháp (synopsis nguyên văn):**
  > `git remote set-url [--push] <name> <newurl> [<oldurl>]`
  >
  > Changes URLs for the remote. (Đặt lại URL cho remote tên `<name>` — chính là lệnh
  > dùng để đóng bug này.)

## 2. Liên hệ tới bug của mình
`.git/config` giữ URL của remote `origin`. Đổi tên repo trên server **không** đụng file
này; phải tự chạy `git remote set-url origin <url-mới>` để cập nhật. Đây đúng là cách
đã sửa KI-GIT-0003.

## 3. Snippet + vị trí (hiện tại)
`.git/config`, mục `[remote "origin"]`:
```
[remote "origin"]
    url = https://github.com/chinart123/STM32F103C8T6.git   # SAU khi sửa (TRƯỚC là …108…)
```
Kiểm tra: `git remote -v`.
