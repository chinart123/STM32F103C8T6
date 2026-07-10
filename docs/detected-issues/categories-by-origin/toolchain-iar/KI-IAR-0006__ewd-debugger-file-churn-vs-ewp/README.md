# KI-IAR-0006 — `test.ewp` vs `test.ewd`: một project IAR, HAI cuốn sổ cấu hình

**Triệu chứng:** sau khi tạo build configuration mới (`Debug_Vendor`) trong GUI,
git báo modified **cả hai** file: `test.ewp` (chủ đích sửa) và `test.ewd`
(không ai đụng tay). `git diff --stat` cho thấy `.ewd` phình **+1.481 dòng**.
Không có lỗi compile/build nào — hiện tượng thuộc về pha **run** (Download &
Debug) và vệ sinh git. Xem `error.log` cho output nguyên văn.

## Chuỗi nguyên nhân
1. IAR tách cấu hình project thành các file theo VAI, cùng tên khác đuôi:
   - `test.eww` — workspace (gom các project);
   - `test.ewp` — **BUILD**: include path, define, optimize, file nào được dịch;
   - `test.ewd` — **DEBUGGER**: driver nào (`OCDynDriverList = STLINK_ID`),
     flash loader nào (`FlashSTM32F10xx8.board`), setting C-SPY, catch fault…
2. Mỗi build configuration có MỘT khối trong `.ewp` **và MỘT khối tương ứng
   trong `.ewd`**. Tạo `Debug_Vendor` trong GUI → IAR ghi khối build vào `.ewp`
   VÀ nhân bản nguyên khối debugger (~1.481 dòng) vào `.ewd`.
3. `.ewd` còn bị IAR ghi lại lặt vặt gần như mỗi lần mở/đóng — nội dung gắn với
   máy cụ thể (probe, đường dẫn `$TOOLKIT_DIR$`…).
4. Hệ quả: chữ `M` cạnh `test.ewd` xuất hiện dai dẳng trong source control và
   gây bối rối ("mình có sửa gì đâu?"), hoặc tệ hơn — bị commit rác theo máy.

**Gốc rễ:** một "project" trong đầu người dùng là MỘT thứ, nhưng trên đĩa IAR
lưu thành NHIỀU file theo vai — và chỉ MỘT trong số đó (`.ewp`) là tài sản
chung đáng version; phần còn lại là trạng thái theo máy.

## Cách xử (đã áp dụng)
- **Chỉ commit `test.ewp`.** `test.ewd` để nguyên trạng thái modified — vô hại
  (chính sách có từ §6, phiên Buổi 11).
- Muốn ẩn hẳn chữ `M`: `git update-index --skip-worktree <path>/test.ewd`
  (bảo git "đừng nhìn file này nữa"; gỡ bằng `--no-skip-worktree`).
- AI của dự án bị CẤM sửa tay `.ewd` (cùng danh sách với `.eww`, device/linker
  option) — chỉ `.ewp` được sửa trong phạm vi include path + đăng ký file.

## Bài học / phòng ngừa
- Gặp file lạ modified sau một thao tác GUI: tra VAI của đuôi file trước khi
  commit — "IDE của tôi vừa ghi gì, thay mặt tôi?"
- Quy tắc phân loại của repo áp dụng đẹp ở đây: cấu hình là tài sản chung
  (build) → version; cấu hình theo máy (debugger/probe) → local.
- Doc giáo dục chi tiết (snippet XML thật + hình giải phẫu bộ ba file):
  `Take note quá trình học thanh ghi/Buoi_12.0_test_package_No.0/Giai_thich_ewp_vs_ewd.md`

## Trạng thái
`mitigated` — hiện tượng được hiểu rõ, chính sách commit đã chốt, có đường
skip-worktree cho ai muốn panel sạch.
