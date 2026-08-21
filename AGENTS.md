# Working in this repo

This is a personal systems-programming curriculum: notes (конспект) + hands-on labs in C (and some hand-written x86-64 assembly), building toward understanding how nginx and Node.js work under the hood, and how Docker, Kubernetes, and OS networking fit together. If you're an agent working in this repo — any agent, not a specific one — here's how to do it well.

## Communication

Before answering, restate the user's message in natural, conversational American English. They sometimes write in Russian, or mix Russian words into English — translate and clean this up first, every message, without flattening their meaning or trimming detail. Apply this to every message in this repo, not just the obviously non-native ones.

## Depth

- Default to real depth, not shallow "primer" coverage. When a question surfaces a topic deeper than what was planned, treat it as a real candidate for expanding the curriculum (a new module: notes + lab) rather than a quick aside.
- Prefer hands-on verification over assertions — actually run the commands (`gcc`, `readelf`, `objdump`, `nm`, `gdb`, `strace`, `/proc` inspection, etc.) and show real output, rather than describing what "should" happen.
- Sequencing/placement of new material in the roadmap is worth asking about; the depth of the content itself is not something to undersell once it's scoped.

## Language & curriculum choices

- Labs are written in C (plus some hand-written assembly), not Rust. Rationale: nginx and Node's runtime layer (V8/libuv) are C/C++, and the explicit goal is learning the raw OS interface (syscalls, memory layout, POSIX, ELF) directly, rather than through Rust's abstraction on top of it. Revisiting Rust later, after the fundamentals, is a reasonable plan, not something ruled out.
- `README.md`'s roadmap is the current source of truth for module order and numbering. It has already been renumbered once as modules got inserted mid-curriculum (CPU architecture and virtual memory were added after toolchain/processes were already underway) and will likely keep shifting — check it before trusting any other document's module list.

## Repo structure & conventions

- `notes/<NN-module-name>/notes.md` — the structured concept write-up.
- `notes/<NN-module-name>/qa.md` — only for a module that involved a genuine back-and-forth (not just reading the note): the actual questions asked, wrong turns/corrections made along the way, and the real commands/output that settled things. Don't create one preemptively — add it once a real discussion actually happens.
- `labs/<NN-module-name>/` — one directory per module:
  - `instructions.md` — what to implement/do, and how to verify it
  - one or more source files (`.c` or `.s`) as **skeletons with `TODO` markers** — don't hand over a finished solution; filling the gaps is the point of a lab
  - a `Makefile`
  - Exception: a lab about *using a tool* (e.g. `gdb`, `strace`) rather than *writing code* can be fully working/ready-to-run, since the exercise is driving the tool, not filling in gaps.
- Read the note first, then work the labs in order — later labs in a module build on earlier ones.
- Toolchain on this machine: `gcc`, `make`, `gdb`, and `strace` are all installed. They weren't present when the repo started (had to be installed mid-curriculum) — if a fresh environment is missing one, that's expected, not a repo problem.

## Verifying before claiming something works

- Compile-check every skeleton/example before describing it as working — `gcc -Wall -Wextra` at minimum. Unused-variable/parameter warnings on unfilled `TODO`s are expected and fine; real errors are not.
- Clean up build artifacts (`make clean` / `rm`) after verifying.
- Run `git status` before any broad change, and before anything that could discard uncommitted work — nothing in this repo is committed yet as of this writing, but don't assume that stays true.
