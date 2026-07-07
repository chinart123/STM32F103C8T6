# AGENTS.md — Project Law (Claude, Gemini, DeepSeek, any brain must obey)

> This is the **short spine**. Detail lives in the files it points to. Read this
> first, every fresh session. The chat is disposable; these files are the truth.

## Identity
STM32F103C8T6 (Blue Pill), **bare-metal C**, **NO HAL / NO LL** — raw register
access only. A personal learning project. Real firmware lives in `C&C++/`;
hand-written learning notes live in `Take note quá trình học thanh ghi/`.

## Step 0 — bootstrap (do this before anything else)
1. Read this file.
2. Read `STANDARD.md` (the coding standard) before writing or editing any code.
3. Read `Log-and-Report-writing-tools/logs/bug_log.md` (recent bugs & fixes).
4. Skim the file(s) you're about to touch. Then plan, then make one small edit.

## Hard rules — never break
1. **Encoding = UTF-8**, always. Vietnamese (ă â đ ê ô ơ ư + tone marks) must
   render. On PowerShell write with `-Encoding utf8`.
2. **Bilingual:** code comments in **English**; learning notes, `bug_log`, and
   reports in **Vietnamese**.
3. **Follow `STANDARD.md`.** Do not invent a new style. Keep the project's
   `BUNION`/`RSTRUCT` register-map DSL and bit-band / BSRR idioms.
4. **Do not modify `C&C++/` or `Take note …/` unless explicitly asked.** Create
   new files or append; never silently rewrite the user's history.
5. **Log every bug you fix** by running the bug-logger skill — never make the
   user hand-write notes. Capture *real runtime data*, not recollection.
6. **AI machinery stays local.** Everything under `Log-and-Report-writing-tools/`
   is git-ignored and is never committed or pushed.
7. **Never auto-publish unverified firmware.** Commit to a `wip/` branch or tag
   `[unverified-build]`; `main` only advances after the user confirms on-board.
8. **Feed slices, never whole files.** Grep/extract the relevant lines of a
   datasheet or runtime log before reading — keep context (and tokens) small.

## The hardware boundary
The AI owns **"does it BUILD"** (`iarbuild`). The user owns **"does it WORK"**
(Download & Debug in IAR). Hardware truth crosses back only via a captured
`runtime.log` — read that, don't guess from the user's memory.

## Commit convention (§ sections)
Commits are numbered **per branch**, initial import = **§0**, counting up §1, §2,
§3… — no zig-zag across branches. Every commit subject begins with its `§<n>` marker.
When a commit's payload lives in a subfolder, its full message is also saved as a file
**inside that folder** and used verbatim (`git commit -F`): `_commitmsg_<n>.txt` on
`main`, `_note_commitmsg_<n>.txt` on a notes branch. (Top-level / trivial commits may
skip that file to keep the repo root clean — the `§` subject + commit-object message
suffice.) **Each §-commit is self-contained: it holds only what belongs to its own
stage** — no other stage's numbers or content bleeding into its message or files.
These are log/documentation commits (not compile/run), so clean per-stage splitting
never breaks the build.

## Pointers
- Coding standard → `STANDARD.md`
- Skills (each is a thin wrapper over a CLI script) → `Log-and-Report-writing-tools/skills/`
- Bug history → `Log-and-Report-writing-tools/logs/bug_log.md`
- Datasheets (local) → `docs/datasheets/`  ·  archived design → `docs/archive/`

## Continuity
State = `AGENTS.md` + `STANDARD.md` + `bug_log.md` + the code + git history.
Any brain, any fresh chat, reads these and is caught up. Session-resume IDs do
**not** cross brains — these files are the only handoff.

## Working loop
Research (read real files) → Plan (say it simply, learner-friendly) → one small
correct edit → `iarbuild` → capture runtime data → log any bug → stop for review.

## Setup footprint (per machine)
`pdftotext` (poppler) for datasheet lookup; `pyserial` only if UART capture is
used. Node is present for WaveDrom. No venv until the report skill is added.
