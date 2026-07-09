# SIZE-CHECK — kiểm tra chéo giới hạn GitHub 100MB/file

> Mục đích: xác nhận không file nào trong kho `STM32CubeF1\` (nhập nguyên trạng
> từ hãng) vượt hard-limit **100MB/file/push** của GitHub (50–100MB chỉ cảnh báo).
> Đo ngày 2026-07-09, sau khi clone `--recursive --depth 1` và xóa `.git` con.

## File lớn nhất của từng thư mục gốc

| Thư mục | File lớn nhất | Dung lượng |
|---|---|---|
| `Drivers\` | `CMSIS\Lib\ARM\arm_cortexM3b_math.lib` (thư viện DSP biên dịch sẵn) | **13.06 MB** |
| `Middlewares\` | `Third_Party\LwIP\doc\doxygen_docs.zip` | 2.01 MB |
| `_htmresc\` | `STM32Cube_2020.bmp` | 1.77 MB |
| `Documentation\` | `STM32CubeF1GettingStarted.pdf` | 0.52 MB |
| `Utilities\` | `Media\Audio\audio.bin` | 0.18 MB |
| `Projects\` | `STM32CubeProjectsList.html` | 0.12 MB |
| `.github\` | `ISSUE_TEMPLATE\bug_report.yml` | ~0 MB |

## Kết luận

- **File lớn nhất toàn kho: 13.06 MB** — dưới 100MB (hard-limit) và dưới cả 50MB
  (ngưỡng cảnh báo). ✅ An toàn tuyệt đối để commit + push.
- Tổng dung lượng kho: ~184 MB (chấp nhận theo quyết định "repo học tập, không
  tối ưu lưu trữ").
- Lưu ý cho tương lai: giới hạn 100MB tính trên TỪNG file trong MỘT lần push,
  không cộng dồn theo thời gian; nhưng file đã lỡ commit thì nằm lại trong git
  history kể cả khi xóa — file mới thêm vào kho này phải qua scan trước khi commit.
