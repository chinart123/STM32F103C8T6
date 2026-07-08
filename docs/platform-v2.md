# PLATFORM v2 — "Repo-as-Platform" + Figure Skill 3 lớp (bản phê duyệt)

> **File này để làm gì:** bản thiết kế platform đã được user phê duyệt (2026-07-08).
> Bất kỳ AI model nào (Claude, ChatGPT, Gemini, DeepSeek…) đọc `AGENTS.md` +
> `STANDARD.md` + file này là đủ ngữ cảnh vận hành dự án — chat cũ là đồ bỏ,
> file là sự thật.
>
> Ngôn ngữ: prose tiếng Việt, token kỹ thuật giữ tiếng Anh. UTF-8.

**Chú giải icon:** ✅ thêm mới · ❌ bỏ ra · ⚠️ lưu ý · ⭐ điểm "ăn tiền".

---

## 0. Mục tiêu

- **Tối thượng:** platform 10 điểm từ STM32F103 bare-metal; khi đổi MCU
  (ESP32-S3, IDE mới) chỉ cần gọi AI vào edit — kế thừa mọi bài học cũ, loại trừ
  điểm yếu cũ, tiếp tục cải tiến. Áp dụng được cho **nhiều AI model**.
  ⚠️ Về mặt git, lần đổi MCU khớp với **chiến lược comeback**: `main` là xương
  sống vĩnh viễn + tag mốc kỷ nguyên (`git tag -a comeback/N`), note branch mới
  cho mỗi lần quay lại — branch off **đỉnh main lúc đó**, đánh số `C2§0, C2§1…`
- **Gần nhất:** báo cáo phiên làm việc **cho AI đọc trước** nhưng vẫn
  *human-eyecatching*; kho `docs/detected-issues/` ngày càng phong phú để AI mới
  vào là bắt kịp. Báo cáo phải **ra đều, đẹp, có tính kế thừa**.
- **Chuẩn nghiệm thu thẩm mỹ (north star):** luận văn tốt nghiệp cũ của user
  (file local, không track) — khi output tự sinh đặt cạnh nó mà không thấy ngượng
  (hơi hướng scientific research / đồ án) thì đạt 10 điểm. Chỉ tham khảo *độ đẹp
  mắt*, không copy cấu trúc kỹ thuật.

## 1. Định dạng spine: Markdown, không phải docx

| Hạng mục | Phán quyết |
|---|---|
| Tự sinh OOXML docx thesis-grade (field `TOC`/`SEQ`, `styles.xml`…) | ❌ **HỦY** — tối ưu cho sai người đọc (AI đọc docx rất đắt token); bảo trì vô tận; máy không có Word để verify |
| md2docx / docxgen **v1** (script local) | ⚠️ giữ nguyên, **đóng băng** — chỉ làm cầu export khi cần bản Word trao tay |
| Graphviz auto-layout | ⚠️ hạ cấp: chỉ làm nháp nhanh AI-đọc-AI, KHÔNG dùng cho hình báo cáo |
| WaveDrom cho bit-field thanh ghi | ✅ giữ nguyên vai trò |
| Mọi hard rule `AGENTS.md`, luật §, song ngữ | ✅ giữ nguyên tuyệt đối |

## 2. Kiến trúc tổng thể (text structure)

```
<repo-root>/                                   ← REPO CHÍNH LÀ PLATFORM ⭐
│
├── AGENTS.md                                  ← spine, mọi brain đọc là bắt kịp
├── STANDARD.md                                ← chuẩn code C
├── docs/
│   ├── platform-v2.md                         ← file này (bản thiết kế đã duyệt)
│   └── detected-issues/                       ← kho lỗi public
│       └── categories-by-origin/…/meta.yml
│           └── ✅ trường `scope:`             ← universal | toolchain | mcu-specific | process
│                                                ⭐ chìa khóa chuyển MCU: lọc
│                                                  scope != mcu-specific là kế thừa sạch bài học
│
├── C&C++/                                     ← firmware — VỊ TRÍ BẤT KHẢ XÂM PHẠM (IDE trỏ vào)
├── Take note quá trình học thanh ghi/          ← note tay của user — VỊ TRÍ BẤT KHẢ XÂM PHẠM
│
└── Log-and-Report-writing-tools/              ← AI machinery, git-ignored trên main (hard rule #6)
    │                                            ⚠️ các phần đáng giữ được `git add --sparse -f`
    │                                            lên NOTE BRANCH (worktree riêng) → push remote
    │                                            private `notes` → không mất khi đổi máy
    ├── logs/bug_log.md                        ← bug firmware-logic, local-only
    ├── reports/
    │   ├── INDEX.md                           ← ✅ 1 dòng/báo cáo — AI đọc index trước ⭐
    │   └── report_buoi_N.md                   ← ✅ BÁO CÁO PHIÊN chuẩn hóa (thay docx)
    │       ├── YAML frontmatter (máy đọc):     session, mcu, toolchain, request,
    │       │                                   files_touched, issues[KI-…], outcome
    │       └── thân bài (người đọc):           Yêu cầu → Thay đổi → Build/Run →
    │                                           Issues đã gặp → Bài học
    └── skills/
        ├── report-builder/
        │   ├── docxgen.py + md2docx.py        ← v1 đóng băng ⚠️
        │   └── ✅ report-assembler (Python stdlib)
        │       └── gom issue-record + runtime.log → sinh report_buoi_N.md
        │           ⭐ báo cáo được SINH RA, không viết tay → "ra đều" là bảo đảm cơ khí
        └── figure/                            ← ✅ SKILL MỚI — 3 LỚP
            ├── FIGURE-STANDARD.md             ← Lớp 2: STYLE CONTRACT ("STANDARD.md cho hình")
            │     · file .md thường — MỌI model đọc được, không độc quyền hãng nào
            │     · legend bắt buộc · trái→phải, feedback vòng dưới
            │     · ký hiệu khoa học ⊕ / G(s) / Hy Lạp · quy ước màu, mũi tên, caption
            ├── Lớp 1: ENGINES
            │     · TikZ (Tectonic) → hình human-facing chuẩn paper
            │     · matplotlib → đồ thị số liệu từ runtime.log
            │     · WaveDrom → bit-field thanh ghi
            │     · Graphviz → nháp nhanh AI-facing (không vào báo cáo)
            ├── Lớp 3: exemplars/ ("RAG" bản local) ⭐
            │     · hình user duyệt OK → lưu CẢ source .tex/.py LẪN .png
            │     · hình mới = biến tấu exemplar gần nhất, không sáng tác từ 0
            └── SELF-INSPECT LOOP ⭐
                  · render PNG → AI TỰ NHÌN LẠI ảnh → checklist (chồng chữ?
                    thiếu legend? mũi tên gãy?) → tự sửa, đạt mới đưa user
```

## 3. Luồng làm việc một phiên

```
user gõ yêu cầu
   → AI sửa code / config           (STANDARD.md + detected-issues soi đường)
   → build                          (AI own "does it BUILD"; thực tế user có thể
   │                                 build trong IAR và dán lỗi về)
   → user Download & Debug          (user own "does it WORK" — phần cứng)
   → runtime.log quay về AI         (hardware truth, bắt qua C-SPY/SWD)
   → lặp đến khi OK
   → bug mới? → bug-logger → detected-issues (+ scope tag)
   → report-assembler SINH report_buoi_N.md (+ hình từ figure skill)
   → user review → CHECK CẶP §-commit (bất biến B2) → commit main
   → sync note branch → push main→origin · push note→notes
```

⭐ AI đời sau (bất kể hãng nào) chỉ cần đọc `AGENTS.md` → `reports/INDEX.md` →
frontmatter vài báo cáo → `detected-issues` lọc theo `scope` — vài nghìn token
là nắm toàn bộ lịch sử dự án. Đúng luật "feed slices".

## 4. Footprint cài thêm (per machine)

| Công cụ | Vai trò |
|---|---|
| ✅ Tectonic (1 exe) | compile TikZ → PDF |
| ✅ poppler đủ bộ (`pdftocairo`) | PDF → PNG cho self-inspect |
| ✅ matplotlib (pip, no venv) | đồ thị số liệu |
| Node + WaveDrom (đã có) | bit-field |
| ❌ KHÔNG Java / Chromium / LibreOffice | footprint nhẹ |

⚠️ Tectonic lần chạy đầu tự tải package qua mạng — cần internet một lần.

## 5. Các quyết định đã chốt (user, 2026-07-08)

1. **exemplars/** đặt tại `Log-and-Report-writing-tools/skills/figure/exemplars/`,
   backup bằng `git add --sparse -f` lên note branch → remote private `notes`.
2. **Báo cáo cũ (docx §6/§7) = di sản** — giữ nguyên chỗ, không đụng; `INDEX.md`
   bắt đầu từ báo cáo `.md` đầu tiên của platform mới.
3. **Trường `scope:`** thêm vào KB bằng một §-commit docs riêng, tự chứa trên main.
4. **Đa model:** style contract là file `.md` thường (`FIGURE-STANDARD.md`);
   SKILL.md của Claude chỉ wrap + trỏ vào.

## 6. Nguyên tắc quản trị & BẤT BIẾN ĐẦU RA ⭐

User giao quyền **"thư ký cấp cao"**: AI được tự suy nghĩ, tự đề xuất, tự thực thi
và **linh hoạt thay đổi quy trình** (mục 3) khi gặp requirement/issue mới — user chỉ
can thiệp khi có issue nặng ảnh hưởng đến rules. Đổi lại, các **bất biến đầu ra**
sau là bất di bất dịch:

- **B1 — main sạch kiểu CV:** tại HEAD của `main` chỉ tồn tại đúng MỘT file
  `_commitmsg_N.txt` (của commit §N mới nhất); commit nào thêm file message mới thì
  `git rm` bản cũ TRONG CÙNG commit. Note branch ngược lại: giữ đủ toàn bộ
  `_note_commitmsg_*.txt`.
- **B2 — main ↔ note song song:** mỗi phiên kết thúc bằng một **CẶP commit khớp
  nhau**: main §N (code/KB, message tiếng Anh) ↔ note §N' (report đúng phiên đó,
  message tiếng Việt). Trước khi push phải kiểm tra cặp tồn tại và cùng nói về một
  phiên/feature. Sai lệch lịch sử (§5–§7 cũ) giữ nguyên làm di sản; bất biến áp
  dụng từ nay về sau.
- **B3 — vị trí bất khả xâm phạm:** `C&C++/` và `Take note quá trình học thanh ghi/`
  không bao giờ bị di chuyển/đổi tên — IDE trỏ đường dẫn vào đó.
- **B4 — ba cái gật riêng biệt:** edit ≠ commit ≠ push; mỗi commit và mỗi push đều
  chờ user yêu cầu đúng bước đó.
- **B5 — chống "sửa 1 rule, vỡ cả quy trình":** mỗi luật chỉ sống ở ĐÚNG MỘT file
  (`AGENTS.md` / `STANDARD.md` / `FIGURE-STANDARD.md` / file này); tài liệu khác
  chỉ **trỏ tới**, không chép lại. Đổi một rule = sửa một file.

## 7. ⚠️ Lưu ý triển khai

- TikZ compile chậm hơn Graphviz (~vài giây/hình) — chấp nhận, hình báo cáo không
  sinh hàng loạt.
- Self-inspect loop tốn thêm token mỗi hình — chấp nhận, ưu tiên chất lượng lâu dài.
- Hình AI-đọc-AI (nháp Graphviz) KHÔNG cần qua style contract.
- Export docx v1: TOC là hyperlink tĩnh — chấp nhận, docx chỉ là cầu trao tay.
