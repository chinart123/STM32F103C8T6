# IAR Embedded Workbench IDE Project Management and Building Guide — mục "File types"

- **Tài liệu:** IAR EWARM — *IDE Project Management and Building Guide*
  (bản PDF đi kèm cài đặt: `<IAR>\arm\doc\EWARM_IDEGuide.ENU.pdf`, và mở được
  từ IDE: Help > Information Center > Project management and building guide).
- **Điểm liên quan (tóm tắt ý, không phải trích nguyên văn):** bảng "File
  types" của guide liệt kê vai từng đuôi file do IDE sinh ra, trong đó:
  `*.eww` = workspace file; `*.ewp` = project file chứa **build settings**;
  `*.ewd` = file chứa **C-SPY debugger settings** của project; `*.ewt` =
  settings của C-STAT/C-RUN. Guide cũng ghi rõ nhóm file settings được IDE
  cập nhật tự động khi làm việc.

## Liên hệ với bug này
Đây là văn bản "chính chủ" xác nhận `.ewd` thuộc miền DEBUGGER (pha run/nạp),
tách khỏi miền BUILD của `.ewp` — nền của chính sách: version `.ewp`, để
`.ewd` local. Hiện tượng +1481 dòng khi tạo config mới là hành vi thiết kế
(mỗi configuration có khối C-SPY riêng trong `.ewd`), không phải lỗi.
