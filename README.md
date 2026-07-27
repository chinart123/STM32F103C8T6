# STM32F103C8T6 — Industrial Library Structure Learning

An in-depth research and analysis project on the architecture of STMicroelectronics' industrial software libraries (for the STM32F103C8T6 microcontroller series). 

**Core Objective:** The project's methodology does not aim to reject the manufacturer's libraries, but rather uses direct register-level programming (bare-metal C) as a deconstruction tool. The process of rebuilding the modules from scratch helps to fully understand the design philosophy, source code organization, and foundational architecture of the standard HAL/LL libraries.

**Project Approach Guidelines:**
*   **Step 1:** Read the `AGENTS.md` file to thoroughly understand the comparison rules and source code construction standards.
*   **Step 2:** Refer to `docs/platform-v2.md` to understand the overall architecture (platform blueprint) being simulated.
*   **Step 3:** Access `docs/detected-issues/` to look up technical issues encountered during hardware deconstruction and their resolutions.

---

## 📦 Project Components Details

The project implements a "Dual-track" learning model, dividing the source code into clear comparison areas:

*   **`C&C++/`**
    *   Directory containing 100% self-developed firmware source code at the register level (bare-metal).
    *   Strictly adheres to the NO HAL / NO LL rule in the main firmware core to serve core learning purposes.

*   **`Take note quá trình học thanh ghi/`**
    *   Personal notes documented during each practical library analysis session (in Vietnamese).

*   **`docs/`**
    *   Stores architectural blueprints and the issue repository (detected-issues).
    *   Structurally organized to facilitate engineer reference and AI analysis tasks.

*   **`Manufacturer_Package/` — Original Reference Data (Provenance)**
    *   This area strictly contains no self-written code; it only stores the original source code from STMicroelectronics to serve as a "guiding compass" for architectural reference. Includes:
    *   `STM32CubeF1/`: An exact clone from the manufacturer's official repository (including 3 submodule drivers). The source code complies with the **BSD-3-Clause** license, and the original `LICENSE.md` file is preserved. The `.git` metadata has been removed so the project can be managed independently.
    *   `No.0_C&C++_Industrial_Draft/`: This is the result of the process of manually "cloning" the drivers based on analyzing the structure of the manufacturer's library. It contains the original ST files (retaining 100% of names and content) but **moved** into a hierarchical directory model identical to `C&C++/`. 
        *   *Purpose:* Dual-track learning — writing the manual bare-metal version in `C&C++/` first, then directly comparing it with the manufacturer's API organization here. 
        *   *Progress Mapping:* Manufacturer files are only "imported" into this Draft directory when reached in the learning roadmap. The commit history (git history) here accurately reflects the progress of absorbing the manufacturer's design philosophy. Each major restructuring will create a new batch (`No.1_`, `No.2_`...).

---

## 🗂️ Overview of Directory Tree Structure

```text
.
├── AGENTS.md                               # Project rules and standards
├── C&C++/                                  # Self-developed firmware (100% bare-metal)
├── docs/                                   # Technical documentation, architecture, and issue repository
│   ├── detected-issues/                    # Repository of encountered issues
│   └── platform-v2.md                      # Platform architecture blueprint
├── Manufacturer_Package/                   # Original reference data (Provenance)
│   ├── No.0_C&C++_Industrial_Draft/        # Restructured driver from manufacturer's library
│   └── STM32CubeF1/                        # Exact clone from ST
└── Take note quá trình học thanh ghi/      # Personal learning notes
```
