# STM32 Bare-Metal — AI Assistant Kickoff Brief

> **What this file is.** A self-contained brief you paste into a **new chat** — works for
> **Claude Code _or_ Google Antigravity/Gemini**. It tells whichever assistant reads it exactly
> what the project is, the hard rules both models must obey, and the 3 small tools to build
> (simplest first). Companion background: [`STM32_BareMetal_Deep_Overview.md`](./STM32_BareMetal_Deep_Overview.md).
>
> **Encoding: every file in this project is UTF-8** (so Vietnamese text — ă â đ ê ô ơ ư and
> tone marks — never corrupts). On Windows PowerShell, always write with `-Encoding utf8`.

---

## 0. The one idea that makes two AI models safe

You will use **two brains** — Claude and Gemini — so that when one runs out of tokens, you
switch to the other and **keep going without losing progress**.

That works because **your progress is not stored in the AI's memory. It is stored in three
files on disk:**

| File | Role | Plain words |
|---|---|---|
| `AGENTS.md` | **The top law** | The rules both models must follow (your style, UTF-8, no HAL…) |
| `bug_log.md` | **The history** | Every bug + fix, auto-recorded — what you used to hand-type in `Note.txt` |
| your `Buoi_*` code | **The work** | The actual firmware |

Any model, in any fresh chat, reads those three files and is **instantly caught up**. The chat
is disposable; the files are permanent. That is your continuity.

> **Not needed yet:** MCP (tool connectors) and RAG (document search) are *later* extras.
> Right now you need **one rules file + two small skills**. Nothing more. Don't let anyone
> add a vector database or a Python venv sandbox to this project unless a real need appears.

---

## 1. The project

- **Board:** STM32F103C8T6 (Blue Pill), **bare-metal C**, **no HAL / no LL** libraries.
- **Style:** direct register mapping via nested `struct`/`union`; atomic ops via `BSRR`/`BRR`
  and bit-banding; enums that match ST's reference-manual bit-pairs; non-blocking SysTick /
  interrupt state machines.
- **Layout:** a learning progression `Buoi_1 … Buoi_10` (each = one session), plus hand-written
  `Note.txt` / `what_change.txt` files.
- **Language:** comments and notes are in **Vietnamese**.
- **Toolchain:** IAR / Keil style (`__root __no_init`, `startup_stm32f10x_md.s`).

---

## 2. What I want the AI to do (3 goals)

1. **Write code in MY pattern** — same struct/union register style, same Vietnamese comment
   style. It must learn from my real files, not invent its own style.
2. **Auto-log the buggy code** — a Python tool that grabs the exact broken snippet and appends
   it to `bug_log.md`, so I stop hand-writing `Note.txt`.
3. **Make reports** — turn my notes / bug log into a formatted **.docx and .pdf** report.

More tools will come later, so keep everything **small and modular** — one folder per tool,
never one giant blob.

---

## 3. THE TOP LAW — drop this in as `AGENTS.md`

> This is the single file **both Claude and Gemini read**. Claude Code also reads `AGENTS.md`
> directly; if you prefer a `CLAUDE.md`, make it one line: `@AGENTS.md`. Antigravity reads
> `AGENTS.md` at the project root natively. **Same law, both brains.**

```markdown
# AGENTS.md — Project Law (Claude AND Gemini must obey this)

## Identity
STM32F103C8T6 bare-metal C. NO HAL, NO LL. Raw register access only.
This is a personal learning project (sessions: Buoi_1 … Buoi_N).

## Hard rules — never break
1. ENCODING: every file is UTF-8. Vietnamese text must render correctly.
2. LANGUAGE: all comments and notes in Vietnamese, matching the existing tone.
3. STYLE: match the user's existing pattern — nested struct/union register maps,
   enums that mirror ST reference-manual bit-pairs, BSRR/BRR + bit-banding for
   atomic ops, non-blocking SysTick / interrupt state machines. NO blocking
   for-loop delays in new code unless the user asks.
4. NEVER invent a new style. Before writing, READ 2–3 of the user's real files
   (prefer the cleanest recent session) and copy that exact style.
5. DO NOT modify existing Buoi_* files unless explicitly asked. Create new files
   or append; never silently rewrite the user's history.
6. LOGGING: whenever you fix or diagnose a bug, record it by running the
   bug-logger tool (skills/bug-logger). Do not make the user hand-write notes.
7. SMALL EDITS: surgical changes, match surrounding code, explain in plain
   Vietnamese-friendly terms — the user is learning.

## Two-brain continuity
The chat is disposable. State lives in AGENTS.md + bug_log.md + the code.
On a fresh session, READ those first before doing anything.

## Working loop
Research (read the real files) -> Plan (say it simply) -> small correct edit ->
log any bug -> stop and let the user review.
```

---

## 4. The build plan — simplest first

Do these in order. Each is self-contained so a half-finished session never blocks the next.

### Phase 1 — Foundation (fast, ~1 short session)
- Read **2–3 real files** (e.g. a clean `gpio.c/h` + a `main`) to learn the style.
- Write **`AGENTS.md`** (section 3 above, filled in with concrete observations from those files:
  brace style, naming, how comments are phrased).
- Create the folder skeleton (section 6). **No code tools yet.** Just the law + empty homes.

### Phase 2 — The bug-logger  ⭐ (your biggest pain)
A small Python script `logbug.py` in `skills/bug-logger/`. Two ways to fire:

- **You run it:**
  ```
  python logbug.py "main(10.1).c" --lines 40-55 \
     --symptom "TIM1_UP interrupt never fires" \
     --cause  "quên set BIT25 trong ISER0" \
     --fix    "*((unsigned long*)0xE000E100) = BIT25;"
  ```
- **The AI runs it for you** the moment it fixes a bug (rule 6 in the law).

It pulls the **real lines from the file**, timestamps them, tags the session, and **appends**
to `bug_log.md` (and a sortable `bug_log.csv`). It never edits your source. Output entry:

```
### 2026-07-07 · Buoi_10 · main(10.1).c  [dòng 40–55]
Triệu chứng:  TIM1_UP interrupt never fires
Nguyên nhân:  quên set BIT25 trong ISER0
Cách sửa:     *((unsigned long*)0xE000E100) = BIT25;

<the exact 40–55 lines, copied from the file>
```

*(Everything written UTF-8.)*

### Phase 3 — The report skill
Turn `bug_log.md` + your session notes into a clean **.docx and .pdf** report (grouped by
Buoi, table of bugs, code snippets formatted). On Claude Code this uses the built-in document
skills; on Gemini, an equivalent skill. Same input files → same report on either brain.

---

## 5. Rules ≠ MCP ≠ RAG (so the jargon stops confusing you)

| Term | What it is | Do you need it now? |
|---|---|---|
| **Rules** (`AGENTS.md`) | The law both models obey | **YES — the only must-have** |
| **Skill** | A folder with a script the AI runs (our bug-logger, report) | **Yes — that's how we build the 2 tools** |
| **MCP** | A connector to an outside tool (GitHub, a database) | No — maybe much later |
| **RAG** | A searchable memory of many documents | No — you don't have a big doc pile |

---

## 6. Folder skeleton (goes at the STM32 project root)

Non-destructive — your `Buoi_*` folders are untouched.

```text
Take note quá trình học thanh ghi/       # (path has spaces + Vietnamese — scripts use relative paths)
├── AGENTS.md                # ⭐ the top law — BOTH brains read this
├── CLAUDE.md                # one line: @AGENTS.md   (so Claude Code picks up the same law)
├── bug_log.md               # auto-generated history (created by the logger)
├── bug_log.csv              # sortable version
├── skills/
│   ├── bug-logger/
│   │   ├── SKILL.md         # tells the AI when/how to use logbug.py
│   │   └── logbug.py        # the Python tool
│   └── report/
│       └── SKILL.md         # notes + bug_log -> .docx / .pdf
└── (Buoi_1 … Buoi_10 — your existing work, untouched)
```

> **Skills on both platforms:** the `skills/` folder is the same format for both. Claude Code
> looks in `.claude/skills/`, Antigravity looks in `.agents/skills/` — so once the tools exist,
> copy (or symlink) `skills/` into whichever of those the brain you're using expects. One-time,
> not per-use.

---

## 7. How to start the new chat

1. Open the STM32 folder (`D:\LIBRARIES\Take note quá trình học thanh ghi`) in Claude Code
   **or** Antigravity.
2. Paste this as your first message:

   > *"Read `STM32_AI_Setup_Kickoff.md`. Then do **Phase 1**: read 2–3 of my real files to
   > learn my style, and write `AGENTS.md`. Show me the `AGENTS.md` before writing any tool.
   > Everything UTF-8, comments in Vietnamese."*

3. After you approve `AGENTS.md`, say *"Phase 2"* to build the bug-logger, then *"Phase 3"*
   for the report skill.

---

## 8. Please confirm (or just correct in the new chat)

- **Where the AI setup lives:** at the root of your existing `Take note…` folder (my assumption)
  — or do you want a fresh clean folder?
- **Order:** Foundation → bug-logger → report (my assumption). Say so if you want the logger first.
- **Reports:** both `.docx` and `.pdf` (my assumption).
```
