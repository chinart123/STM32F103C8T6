# GitHub Docs — Renaming a repository

## 1. Định danh tài liệu
- **Tên đầy đủ:** "Renaming a repository" (GitHub Docs → Repositories)
- **Nhà phát hành:** GitHub, Inc.
- **Phiên bản:** tài liệu web (cập nhật liên tục, không đánh version)
- **Ngày phát hành / sửa đổi:** trang không hiển thị "last updated"; **fetch ngày 2026-07-08**
- **Link:** https://docs.github.com/en/repositories/creating-and-managing-repositories/renaming-a-repository
- **Mục/section liên quan:** hành vi redirect sau khi đổi tên repo
- **Trích đoạn (NGUYÊN VĂN, fetch 2026-07-08):**
  > "All `git clone`, `git fetch`, or `git push` operations targeting the previous
  > location will continue to function as if made on the new location."
  >
  > "However, to reduce confusion, we strongly recommend updating any existing local
  > clones to point to the new repository URL."
  >
  > "If you create a new repository under your account in the future, do not reuse the
  > original name of the renamed repository. If you do, redirects to the renamed
  > repository will no longer work."

## 2. Liên hệ tới bug của mình
Chính hành vi redirect này khiến push tới tên cũ (108) **vẫn chạy** → che mất việc
`.git/config` còn sai. Và chính khuyến nghị "cập nhật clone local" là việc đã làm để
đóng bug. Cảnh báo "redirect có thể ngừng nếu tên cũ bị dùng lại" = **rủi ro chiếm tên**
đã nêu trong README.

## 3. Snippet + vị trí (hiện tại)
Dấu hiệu quan sát được (trích từ `error.log`):
```
remote: This repository moved. Please use the new location:
remote:   https://github.com/chinart123/STM32F103C8T6.git
```
