# GitHub Docs — "Downloading source code archives"

- **Tài liệu:** GitHub Docs → Repositories → Working with files → Downloading
  source code archives.
- **URL:** <https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives>
- **Điểm liên quan (tóm tắt ý, không phải trích nguyên văn):** GitHub nói rõ
  source code archive (nút Download ZIP / tarball, và cả asset tự sinh của
  Releases) chỉ chứa nội dung của chính repository đó — **không bao gồm
  submodule**; muốn đủ submodule phải clone bằng git.

## Liên hệ với bug này
Đây chính là văn bản "chính chủ" xác nhận bẫy 1: `STM32CubeF1` giữ toàn bộ
driver dưới dạng 22 submodule (xem `error.log`), nên ZIP tải về sẽ thiếu sạch
phần ruột quan trọng nhất. Cách lấy đủ: `git clone --recursive` (đã dùng, kèm
`--depth 1 --shallow-submodules` để bỏ lịch sử).

# git-scm.com — Pro Git, chương "Git Tools - Submodules"

- **URL:** <https://git-scm.com/book/en/v2/Git-Tools-Submodules>
- **Điểm liên quan (tóm tắt ý):** submodule là con trỏ (gitlink, mode `160000`)
  ghi "commit hash + URL repo con" — repo cha không chứa nội dung repo con.
  Đây cũng là cơ chế đứng sau bẫy 2: một thư mục có `.git` nằm trong repo khác
  sẽ được git coi là repo nhúng/gitlink thay vì cây file thường.
