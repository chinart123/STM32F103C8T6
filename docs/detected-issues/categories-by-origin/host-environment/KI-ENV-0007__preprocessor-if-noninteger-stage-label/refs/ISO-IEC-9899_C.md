# Tài liệu: ISO/IEC 9899 (tiêu chuẩn ngôn ngữ C) — điều khoản về `#if` và pp-number

**Định danh:** ISO/IEC 9899 "Programming languages — C" (C99/C11/C17, các mục dưới đây
đánh số theo C11 §6.4.8 và §6.10.1; nội dung tương đương ở mọi bản).

## §6.10.1 — Conditional inclusion (điều khiển `#if`)

Trích ý (paraphrase, không phải nguyên văn có bản quyền):
> Biểu thức điều khiển của `#if`/`#elif` phải là một **integer constant expression**...
> Trước khi ước lượng, mọi macro được bung; các định danh còn sót (kể cả từ khóa) được
> thay bằng `0`. Biểu thức sau đó phải hợp lệ như một hằng số nguyên.

**Liên hệ bug:** `#if STAGE == 0_1` → sau bung macro là `#if 0_1 == 0_1`. `0_1` KHÔNG phải
định danh (nên không được thay bằng `0`) mà là một **pp-number** sai → không tạo được
integer constant expression → directive hỏng. IAR biểu hiện bằng `Error[Pe014]`.

## §6.4.8 — Preprocessing numbers (pp-number)

Trích ý:
> Một *pp-number* bắt đầu bằng một chữ số (hoặc dấu `.` rồi chữ số) và có thể nối tiếp
> bằng chữ số, chữ cái, dấu `_`, `.`, và các chuỗi mũ `e+`/`e-`… Một pp-number **bao trùm
> nhiều dạng ký tự hơn** so với các literal số hợp lệ; nó chỉ được kiểm tra thành literal
> hợp lệ (integer/floating) ở **giai đoạn dịch sau**.

**Liên hệ bug:** `0_1` là pp-number **hợp lệ về từ vựng** (nên không bị "invalid character"),
nhưng **không chuyển thành** integer literal nào (chuỗi chữ số có `_` xen giữa không phải
số nguyên). Đây chính là lý do lỗi rơi vào `#if` (nơi cần integer constant) chứ không phải
lỗi token.

## Cách dùng đúng (rút ra)

- Chọn mode bằng **sự tồn tại của macro**: `#ifdef STAGE_0_1` / `#elif defined(STAGE_0_2)`.
  `#ifdef`/`defined()` chỉ hỏi "macro này có được định nghĩa không" → không đụng tới số học,
  nên nhãn dạng nào cũng an toàn.
- Chỉ dùng `#if EXPR == N` khi cả hai vế **chắc chắn là hằng số nguyên** (vd `#if __STDC_VERSION__ >= 201112L`).

## Vị trí liên quan trong repo
- Bản SAI (đã bỏ): `#define STAGE 0_1` + `#if STAGE == 0_1` — xem `../error.log`.
- Bản ĐÚNG (hiện tại): `#define STAGE_0_1` + `#ifdef STAGE_0_1` — xem `../README.md` mục "Vị trí code".
