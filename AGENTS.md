# Working in this repo

This is a personal, multi-domain technical learning curriculum: notes (конспект) plus hands-on labs. If you're an agent working in this repo — any agent, not a specific one — here's how to do it well.

## Communication

Before answering, restate the user's message in natural, conversational American English. They sometimes write in Russian, or mix Russian words into English — translate and clean this up first, every message, without flattening their meaning or trimming detail. Apply this to every message in this repo, not just the obviously non-native ones.

## Depth

- Default to real depth, not shallow "primer" coverage. When a question surfaces a topic deeper than what was planned, treat it as a real candidate for expanding the curriculum (a new module: notes + lab) rather than a quick aside.
- Prefer hands-on verification over assertions — actually run the relevant commands/tools and show real output, rather than describing what "should" happen.
- Sequencing/placement of new material in a track's plan is worth asking about; the depth of the content itself is not something to undersell once it's scoped.

## The model: a flat module pool, plus track plans on top

- **Modules live flat, at `notes/<slug>/` and `labs/<slug>/`**, one per topic, with a permanent, descriptive, numberless slug (`processes`, `virtual-memory`, `cpu-architecture`, ...). A module's slug is chosen once and never changes — it doesn't encode a position in any sequence, so it never needs renumbering when scope shifts or a new track wants to reference it differently.
- **Tracks are curated plans, not owners.** Each track is a single file, `tracks/<name>.md` — an ordered, numbered list of links into the shared module pool, plus any track-specific framing (why this order, what's not built yet, "builds on" notes). A track file owns no `notes/`/`labs/` content of its own.
- **The same module can appear in more than one track's plan.** A module that's genuinely cross-cutting (e.g. sockets are both "OS" and "networking") doesn't need to pick one true owner — it just gets referenced, in whatever order makes sense, by every track's plan that needs it.
- **Cross-references between modules** (a note leaning on a concept from another module) should be a real relative link — `[Processes](../processes/notes.md)` — not a "module N" number, since numbers don't exist in this model.
- The root `README.md` is the index of tracks; `tracks/<name>.md` is a track's own roadmap.

## Tags, for cross-cutting discovery

A module can belong to more than one topic area at once without that needing to be resolved into one category. Add a line right after a `notes.md`'s H1 title: `Tags: #networking #os` — plain inline hashtags, not YAML frontmatter, specifically so the editor's own text search (VSCode's `Ctrl/Cmd+Shift+F`, or `grep`/`rg #networking`) finds every tagged file with zero tooling, and so the tags still work natively if the notes ever get opened in a tag-aware app (Obsidian, etc.). Tag `notes.md` only, not `qa.md` — a module's `qa.md` inherits the same tags as its `notes.md`.

## Conventions within a module

- `notes/<slug>/notes.md` — the structured concept write-up.
- `notes/<slug>/qa.md` — only for a module that involved a genuine back-and-forth (not just reading the note): the actual questions asked, wrong turns/corrections made along the way, and the real commands/output that settled things. Don't create one preemptively — add it once a real discussion actually happens.
- `labs/<slug>/` — one directory per module:
  - `instructions.md` — what to implement/do, and how to verify it
  - one or more source files as **skeletons with `TODO` markers** — don't hand over a finished solution; filling the gaps is the point of a lab
  - a `Makefile` or equivalent build/run instructions
  - Exception: a lab about *using a tool* (a debugger, a tracer, a profiler, ...) rather than *writing code* can be fully working/ready-to-run, since the exercise is driving the tool, not filling in gaps.
- Read a module's note first, then work its labs in order — later labs in a module build on earlier ones.
- Language/tooling choices are documented in whichever track plan(s) reference a module — check there rather than assuming.

## Verifying before claiming something works

- Compile/run-check every skeleton/example before describing it as working. Warnings on unfilled `TODO`s (unused variables, etc.) are expected and fine; real errors are not.
- Clean up build artifacts after verifying.
- Run `git status` before any broad change, and before anything that could discard uncommitted work.
