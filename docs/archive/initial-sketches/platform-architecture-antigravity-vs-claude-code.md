# New-Project Platform Architecture — Google Antigravity vs. Anthropic Claude Code

> **Why this doc exists.** For the last project you had two competing write-ups:
> Gemini produced [`Antigravity_Architecture_Guide.md`](./Antigravity_Architecture_Guide.md),
> which argues *from the Antigravity side* that a native platform beats a hand-built one.
> The three Claude-side docs in this folder
> ([`what-claude-added-to-this-repo.md`](./what-claude-added-to-this-repo.md),
> [`rag-and-markitdown-setup.md`](./rag-and-markitdown-setup.md),
> [`architecture-before-after.md`](./architecture-before-after.md)) describe the same job done
> *by hand on Claude Code*. This file gives you the **full picture of both platforms**, corrects
> a few stale claims on each side, and proposes **one architecture for your new project** that
> is portable across both. Every source is linked at the bottom.
>
> *Facts current as of mid-2026 (Antigravity public preview, v1.20.x). Both platforms move fast —
> re-check the linked docs before you commit to a layout.*

---

## TL;DR — the recommendation

1. **Build the new project platform-agnostic.** The two ecosystems have converged on three
   open standards — **`AGENTS.md`** (rules/context), **`SKILL.md`** (skills), and **MCP**
   (tools). Author to those and the *same* repo runs on Antigravity, Claude Code, Cursor, or
   Windsurf with only a thin per-tool adapter.
2. **The real difference is not "custom vs. native" — it's who runs the plumbing.** Antigravity
   *manages* retrieval + the MCP daemon for you (less to maintain, less to see). Claude Code
   leaves them explicit (more to maintain, full control + auditability). Pick per pillar, not
   per platform.
3. **Gemini's doc is right that the hand-built RAG pipeline is overhead** — but wrong to call
   native retrieval a strict win. You lose reproducibility and visibility. For a CV / portfolio
   project, *visible* engineering is often the point.

---

## The two platforms in one glance

| | **Google Antigravity** | **Anthropic Claude Code** |
|---|---|---|
| **What it is** | Agent-first IDE + platform (VS Code lineage) with an **Agent Manager** to spawn/observe many agents across editor, terminal & browser | Terminal-native agent (also IDE/desktop/web) that edits your repo and runs tools directly |
| **Default model** | Gemini 3 Pro (also Claude Sonnet 4.5 & OpenAI models — model-optional) | Claude (Opus / Sonnet / Haiku) |
| **Context / rules** | **`AGENTS.md`** at repo root (+ user rules) | **`CLAUDE.md`** at repo root (also reads `AGENTS.md`) |
| **Skills** | `SKILL.md` under **`.agents/skills/`** (workspace) or user-global | `SKILL.md` under **`.claude/skills/`** (project) or `~/.claude/skills/` (user) |
| **MCP config** | Central **`~/.gemini/config/mcp_config.json`**, daemon-managed | Project **`.mcp.json`** + user settings; process per session |
| **Retrieval** | **Native** — large context window + filesystem tools (`grep`, `view_file`, `list_dir`); indexing is built-in & abstracted | **Explicit / bring-your-own** — you build a RAG pipeline (as in `ai/rag/`) *or* just let it grep the tree |
| **Orchestration** | First-class: Agent Manager, async multi-agent, **Artifacts** (plans, screenshots, browser recordings) for verification | Subagents (`.claude/agents/`), the `Agent`/workflow tools, hooks |
| **Verification story** | Agent produces Artifacts you eyeball (task lists, browser recordings) | Agent reports tool output; you review diffs / test runs |
| **Config philosophy** | **Zero-config / managed** — platform owns the lifecycle | **Explicit / composable** — you own the lifecycle |
| **Cost (individuals)** | Free in public preview, generous rate limits | Subscription / API-metered |

---

## The five pillars, compared honestly

### 1. Context grounding (the "constitution")
- **Antigravity:** `AGENTS.md` at the repo root is the passive, persistent rulebook. As of
  v1.20.3 it's a **cross-tool standard** — the same file is honored by Antigravity, Cursor,
  Windsurf, *and* Claude Code.
- **Claude Code:** historically `CLAUDE.md` (what your quadcopter repo uses), but Claude Code
  now also reads `AGENTS.md`.
- **Verdict:** This pillar has *already merged*. Gemini's doc frames "modular `.gemini/rules/`
  vs. monolithic `CLAUDE.md`" as an Antigravity advantage — but that's a **file-splitting style
  choice, not a platform capability**. You can split `CLAUDE.md` into `@imports` too. For the
  new project: **write one `AGENTS.md`** and both platforms use it verbatim.

### 2. Skills (repeatable procedures)
- **Antigravity:** `SKILL.md` (YAML frontmatter + instructions) under `.agents/skills/<name>/`
  for a project, or user-global for all projects.
- **Claude Code:** identical shape — `SKILL.md` under `.claude/skills/<name>/` or
  `~/.claude/skills/`. This is the [Anthropic Agent Skills standard](https://github.com/anthropics/skills).
- **Verdict:** **The `SKILL.md` format is the same on both.** Only the *folder* differs
  (`.agents/skills/` vs `.claude/skills/`). A skill like your `drawio-skill` is portable with a
  symlink or a one-line copy. Gemini's doc calls the Claude layout "namespace fragmentation"
  (`.claude/skills/` **and** `ai/skills/`) — that critique is **fair**: you had two skill
  namespaces. The new project should keep **exactly one** skills root.

### 3. MCP (tool connectors)
- **Antigravity:** one central `~/.gemini/config/mcp_config.json`; the platform daemon spawns,
  restarts, and routes each server. Shared by IDE and CLI.
- **Claude Code:** per-project `.mcp.json` + user settings; servers launch per session via
  `npx`/`uvx`.
- **Verdict:** This is Antigravity's **clearest real win** — a managed daemon with
  crash-restart beats "an `npx` process that can silently die," exactly the *lifecycle fragility*
  the `architecture-before-after.md` doc flagged (item ⑦, the markitdown server that couldn't
  start). But Claude Code's **per-project `.mcp.json` is version-controllable and reviewable** —
  you can see, in the repo, precisely which tools an agent may reach. Managed = less breakage;
  explicit = more auditability.

### 4. Retrieval (how the agent finds knowledge)
- **Antigravity:** "Zero-RAG." A very large context window + deterministic filesystem tools;
  indexing is built in and hidden.
- **Claude Code:** two honest options — (a) just let it `grep`/read the tree (works
  surprisingly far), or (b) the explicit pipeline you built (`markitdown → chunk → embed →
  chromadb`), which turns PDFs/datasheets into searchable memory.
- **Verdict:** Gemini's doc oversells this. Native retrieval **removes real maintenance pain**
  (no venv, no lockfile, no stale index — all genuine costs you hit). **But** you lose:
  reproducibility (you can't pin *how* it retrieved), visibility (it's a black box), and the
  ability to swap the parser/embedder. For datasheets an image-based PDF still needs OCR either
  way. **Rule of thumb for the new project:** start with native/grep retrieval; add an explicit
  RAG pipeline *only* when you have a real corpus of documents the agent must cite.

### 5. Orchestration & verification
- **Antigravity:** the headline feature. The **Agent Manager** runs multiple agents
  asynchronously across editor/terminal/browser, and each emits **Artifacts** — task lists,
  implementation plans, screenshots, browser recordings — so you *verify by looking* rather than
  by reading a transcript.
- **Claude Code:** subagents (`.claude/agents/`), the `Agent`/workflow tools for fan-out, and
  hooks — powerful, but verification is more "read the diff / read the test output."
- **Verdict:** If your new project has a **UI or a browser flow**, Antigravity's artifacted,
  visual verification is a genuine step up. For **pure firmware/CLI/library** work (like the
  quadcopter), Claude Code's diff-and-test loop is just as good and more transparent.

---

## Where each side's doc is right — and where it overstates

**Gemini's `Antigravity_Architecture_Guide.md` gets right:**
- Managed MCP lifecycle > hand-rolled `npx`/`uvx` subprocesses (real fragility you hit).
- A single standardized skill schema > two ad-hoc skill namespaces.
- The hand-built RAG pipeline *is* maintenance overhead (venv drift, stale index).

**…and overstates:**
- **"Zero-RAG is strictly better."** It trades maintenance for opacity and non-reproducibility —
  a trade, not a free win.
- **`.gemini/rules/` + `.gemini/skills/` as the Antigravity layout.** Already **stale**:
  current Antigravity uses **`AGENTS.md`** for rules and **`.agents/skills/`** for skills. The
  doc describes an older Gemini-CLI convention.
- **"Monolithic `CLAUDE.md`" as an inherent weakness.** It's a formatting choice; `CLAUDE.md`
  supports imports and both tools read `AGENTS.md`.

**The Claude-side docs get right:**
- Honest about limits (image-PDFs only yield titles; the MCP server needs `uv`).
- Reproducibility as a first-class goal (`requirements.lock`, `skills.json`, source-URL columns).

**…and overstate:**
- The elaborate `ai/` teaching sandbox + hand-built RAG is **more scaffolding than a small
  project needs** — which is exactly why you're restarting. The new project should earn each
  piece of infrastructure, not front-load it.

---

## The strategic insight: the ecosystems are converging

The most useful finding for a *new* project isn't "which platform wins" — it's that the two
have adopted the **same open standards**:

| Concern | Open standard | Antigravity | Claude Code |
|---|---|---|---|
| Rules / context | **`AGENTS.md`** | ✅ (v1.20.3+) | ✅ (reads it) |
| Skills | **`SKILL.md`** | ✅ `.agents/skills/` | ✅ `.claude/skills/` |
| Tools | **MCP** | ✅ managed daemon | ✅ `.mcp.json` |

So you don't have to bet the project on one vendor. **Author to the standards; keep the
platform-specific bits in thin, swappable adapters.**

---

## Proposed architecture for the NEW project

A **portable core** (works on any agent) + **thin per-platform adapters** (each just points the
platform at the shared core). This is deliberately lighter than the quadcopter's `ai/` sandbox —
you add RAG/subagents only when a real need appears.

```text
new-project/
│
├── AGENTS.md                    # ⭐ THE single source of truth for agent context.
│                                #    Both platforms read this. Replaces CLAUDE.md as primary.
│
├── skills/                      # ⭐ ONE skills root (no second namespace).
│   └── <skill-name>/
│       └── SKILL.md             #    Standard format — portable to either platform as-is.
│
├── mcp.servers.json             # ⭐ ONE canonical list of MCP servers you use.
│                                #    Adapters below translate it to each platform's location.
│
├── .platform/                   # Thin, swappable per-tool adapters (the ONLY vendor-specific dir)
│   ├── claude/
│   │   ├── CLAUDE.md            #    one line: "@../../AGENTS.md" (import the shared rules)
│   │   ├── .mcp.json            #    generated from ../../mcp.servers.json
│   │   └── skills → ../../skills   (symlink, or copy on Windows)
│   └── antigravity/
│       ├── notes.md            #    AGENTS.md is already at root; skills → .agents/skills/
│       └── (user MCP lives in ~/.gemini/config/mcp_config.json — machine-global, documented here)
│
├── docs/
│   ├── datasheets/              # component PDFs (as in the quadcopter repo)
│   ├── research/                # papers
│   └── decisions/               # short ADRs: "why native retrieval", "why this skill", …
│
└── src/  tests/  …              # your ACTUAL product — untouched by any of the above
```

**What's shared vs. platform-specific**

| Layer | File | Portable? |
|---|---|---|
| Rules | `AGENTS.md` | ✅ both read it directly |
| Skills | `skills/*/SKILL.md` | ✅ same format; only the folder path differs |
| Tools | `mcp.servers.json` | ⚠️ canonical list is portable; **location** differs → adapter maps it |
| Retrieval | (none by default) | ✅ start native/grep; add explicit RAG only on demand |
| Orchestration | platform-native | ❌ Agent Manager vs. subagents — don't try to abstract this |

**Rules for keeping it portable**
1. **Never** put project rules only in `CLAUDE.md` — put them in `AGENTS.md`, import from
   `CLAUDE.md`.
2. **One** skills root. Symlink it into each platform's expected path; never fork skill content.
3. Keep `mcp.servers.json` as the human-owned truth; treat each platform's file as generated.
4. Add an ADR in `docs/decisions/` **before** adding heavy infrastructure (RAG, subagents) —
   so future-you knows *why* it exists (the thing the quadcopter repo learned the hard way).

---

## Decision guide — when to lean which way

| If your new project… | Lean toward |
|---|---|
| Has a **UI / web / browser** flow to verify | **Antigravity** (Artifacts, browser recordings) |
| Is **firmware / CLI / a library** (like the drone) | **Claude Code** (transparent diff + test loop) |
| Needs **many agents running async** | **Antigravity** (Agent Manager) |
| Needs **auditable, version-controlled tool access** | **Claude Code** (`.mcp.json` in the repo) |
| Must be **reproducible / show your engineering** (CV) | **Claude Code** (explicit, visible plumbing) |
| You want **least maintenance** | **Antigravity** (managed retrieval + MCP daemon) |
| You want to **not choose** | **Both** — the portable core above runs on either |

---

## Links to every doc

**Google Antigravity (Paradigm B — Gemini's source)**
- Platform home — https://antigravity.google/
- Official documentation — https://antigravity.google/docs/home
- Launch announcement (Google Developers Blog) — https://developers.googleblog.com/build-with-google-antigravity-our-new-agentic-development-platform/
- Getting Started (Google Codelabs) — https://codelabs.developers.google.com/getting-started-google-antigravity
- Authoring Antigravity Skills (Codelabs) — https://codelabs.developers.google.com/getting-started-with-antigravity-skills

**Anthropic Claude Code (Paradigm A — my source)**
- Claude Code overview — https://code.claude.com/docs/en/overview
- Agent Skills (Claude platform docs) — https://platform.claude.com/docs/en/agents-and-tools/agent-skills/overview
- Agent Skills standard & examples (repo) — https://github.com/anthropics/skills
- Engineering: "Equipping agents for the real world with Agent Skills" — https://www.anthropic.com/engineering/equipping-agents-for-the-real-world-with-agent-skills

**Cross-platform standards**
- Model Context Protocol (MCP) — https://modelcontextprotocol.io/
- AGENTS.md open format — https://agents.md/

**The source docs this analysis synthesizes (in this folder)**
- [`Antigravity_Architecture_Guide.md`](./Antigravity_Architecture_Guide.md) — Gemini's Antigravity-side write-up
- [`what-claude-added-to-this-repo.md`](./what-claude-added-to-this-repo.md) — plain-English map of the Claude setup
- [`rag-and-markitdown-setup.md`](./rag-and-markitdown-setup.md) — the hand-built RAG pipeline
- [`architecture-before-after.md`](./architecture-before-after.md) — the 7-point optimization changelog
