# Working in this repo

This repo is a general-purpose, conversational learning framework — not tied to any one subject. `config.json` sets the things that make this particular instance concrete: `native_language`, `working_language`, and `subject`. Content is captured as topics that come up in real discussion rather than from a syllabus decided up front, and — separately — the repo tracks a running model of the user themselves (knowledge gaps, strengths, language notes) in `profile/`, functioning as a kind of externalized second brain, not just a topic library.

**Read `config.json` and `profile/*.md` every time, with the same standing as this file** — not just once at the start of a session, and not just when something profile-related comes up. Calibrating tone, depth, and language to the actual user requires knowing their configured languages/subject and their tracked gaps/strengths going in, not discovering them mid-conversation.

If you're an agent working in this repo — any agent, not a specific one — here's how to do it well.

## Communication

**Before answering — every single message, with no exceptions — restate the user's message in natural, conversational `working_language` (per `config.json`).** They sometimes write in their `native_language`, or mix `native_language` words into `working_language` — translate and clean this up first, without flattening their meaning or trimming detail. This applies to every message, not just the obviously non-native-sounding ones, and not just questions — short, direct, instruction-style messages ("do X," "fix Y") need this exactly as much as long explanatory ones. It's easy to let this slide on messages that don't read as needing correction; don't.

**Answer first; repository work follows.** After the required working-language restatement, give the user the actual conversational or technical answer immediately, before starting note updates, profile maintenance, GitHub writes, verification, or other follow-up work. Do not make the user wait for repository housekeeping before receiving the useful answer. When the interface permits progress messages, send the answer first, then perform the repository work afterward in the same turn or concurrently in the background when that is genuinely supported. Do not claim work is happening in the background if the interface cannot continue after the final response; in that case, answer in an early progress message, complete the work, and finish with a brief self-contained confirmation. Skip repository changes entirely for minor clarifications that add nothing meaningful to the living notes.

## The learning flow: conversation first, notes follow along

- Default mode is a normal conversation — explain things, discuss them, follow tangents, go as deep as the question warrants.
- **`notes.md` is a living document, not a request-gated one.** When a question is substantive enough to be its own topic (comparable in scope to `processes` or `cpu-architecture` — not a one-line clarification), write or update that topic's `notes.md` right away, as part of explaining it, without waiting to be asked. As follow-up questions deepen or correct the picture, **revise the note** — reorganize, rewrite, fold new understanding into the existing flow — rather than just appending. The end state should read as the clearest coherent explanation of the topic, not a chronological transcript of how the conversation went; that's what `qa.md` is for.
- A minor one-off clarification that doesn't really rise to "a topic" doesn't need its own file — fold it into whichever note is already being discussed, or leave it as pure conversation. Use judgment; ask if it's genuinely unclear whether something warrants its own topic, rather than guessing and creating clutter.
- A concept that surfaces mid-explanation and turns out to deserve its own writeup (e.g. explaining interrupts leads into CPU registers) becomes its **own separate topic** the same way — right away, not folded into whichever topic happened to surface it.
- **`qa.md` is different: strictly on request.** Record a question and its answer there only when explicitly asked to — never automatically, no matter how substantial the exchange. It's a deliberate, curated record of specific questions the user wanted preserved verbatim, not a byproduct of every conversation.
- Prefer hands-on verification over assertions when something does get written down — when `subject` involves code, that means actually running the relevant commands/tools and showing real output rather than describing what "should" happen; otherwise, verify however is actually appropriate to `subject`.
- Because notes now change shape over time rather than being written once, git history is the only way to see how a note looked before a revision — committing at reasonable checkpoints is worth suggesting periodically, though only the user decides when to actually commit.

## Tracking the user's learning profile

`profile/` holds a running model of the user, maintained the same way `notes.md` now is — updated as things become apparent in conversation, not gated on being asked:

- `profile/gaps.md` — knowledge gaps that show up, each with a concrete recommendation (build a specific topic, revisit once a specific track has content, etc.). Move an entry to "Resolved" once it's genuinely closed, rather than deleting it.
- `profile/strengths.md` — things the user is already strong at, so future explanations can calibrate to that rather than over-explaining or restarting from zero every time.
- `profile/english.md` — despite the name, this tracks `config.json`'s `working_language`, for a user whose `native_language` is whatever `config.json` says. Record all of the following:
  - **Any significant mistake immediately, even on its first occurrence.** A mistake is significant when it materially changes or obscures the intended meaning, uses the wrong grammatical structure in a way worth learning from, or would be notably problematic in normal professional or native conversation. Do not wait for it to recur.
  - A minor mistake once it recurs enough to show a real pattern. Do not promote obvious one-off typos or rushed spelling slips into learning issues.
  - Genuinely useful/idiomatic phrasing worth surfacing — common among native speakers, easy to underuse as a non-native.
  - Vocabulary diversification — a word or pattern the user reaches for often, with alternatives, so their range broadens over time.
  - For any entry where translating into the native language actually clarifies the meaning (an idiom, a nuance a dictionary definition would miss), include the translation plus an example sentence in each language. Don't force a translation where it doesn't add anything (e.g. a pure grammar/form slip).
  - **Vocabulary diversification specifically stays out of the live conversation by default** — record it in the profile, don't lecture about it mid-discussion. Only mention it live if it's actually important enough to interrupt for (rare) — the profile file is where this is meant to accumulate and be reviewed, not the chat.

**Do not let the English profile become only a collection of “juicy” phrases. Significant errors are higher-priority learning data and must be recorded when they appear.**
- Update these quietly, the way you'd update a note — no need to announce every small addition, though it's worth mentioning when something notable gets added. Be honest about confidence: don't record a "recurring" pattern from a single occurrence, and don't invent gaps/strengths that aren't actually supported by the conversation.

## The model: a flat topic pool, plus track plans on top

- **Topics live flat, at `topics/<slug>/`**, one directory per topic, with a permanent, descriptive, numberless slug (`processes`, `virtual-memory`, `cpu-architecture`, ...) — everything about that topic (notes and any hands-on practice) in one place. A topic's slug is chosen once and never changes — it doesn't encode a position in any sequence, so it never needs renumbering when scope shifts or a new track wants to reference it differently.
- **Tracks are curated plans, not owners.** Each track is a single file, `tracks/<name>.md` — an ordered list of links into the shared topic pool, plus any track-specific framing (why this order, what's not built yet, "builds on" notes). A track file owns no topic content of its own.
- **The same topic can appear in more than one track's plan.** A topic that's genuinely cross-cutting (e.g. sockets are both "OS" and "networking") doesn't need to pick one true owner — it just gets referenced, in whatever order makes sense, by every track's plan that needs it.
- **Tracks evolve as topics get created — this is the main way a track grows now, not up-front planning.** When a topic gets created or meaningfully extended, update *every* track its subject matter makes relevant (check its tags), not just one "owning" track — insert it wherever it fits best in that track's order, and reorder existing entries if the new topic changes what order actually makes sense. If a track doesn't exist yet for a topic that clearly needs one (the way a "CPU" track didn't exist until a CPU topic needed one), create it.
- Cross-references between topics should be a real relative link — `[Processes](../processes/notes.md)` — not a "module/topic N" number, since numbers don't exist in this model.
- The root `README.md` is the index of tracks; `tracks/<name>.md` is a track's own roadmap.

## Creating or updating a topic

1. Create `topics/<slug>/notes.md` with the write-up — or, if this deepens an already-existing topic, revise that topic's existing `notes.md` in place instead of creating something new. Do this as part of explaining, not gated on a request.
2. Add a `Tags:` line right under the H1 title (see Tags below).
3. Add a `Prerequisites:` line right after Tags if there are concepts genuinely needed to understand this topic first — link to an existing topic's `notes.md` if one already covers that prerequisite; otherwise name it in plain text (it can become a real link later, once that topic exists too). Skip the line entirely if there's nothing genuinely prerequisite.
4. Update every track the new topic's subject matter makes relevant, per "Tracks evolve..." above. Ask the user if the right placement or ordering isn't obvious, rather than guessing.
5. Add hands-on practice (see "Conventions within a topic" below) only if it genuinely makes sense for this topic *and* the user wants it — plenty of conversational topics are pure concept and don't need any.

## Tags, for cross-cutting discovery

A topic can belong to more than one subject area at once without that needing to be resolved into one category. Add a line right after a `notes.md`'s H1 title: `Tags: #networking #os` — plain inline hashtags, not YAML frontmatter, specifically so the editor's own text search (VSCode's `Ctrl/Cmd+Shift+F`, or `grep`/`rg #networking`) finds every tagged file with zero tooling, and so the tags still work natively if the notes ever get opened in a tag-aware app (Obsidian, etc.). Tag `notes.md` only, not `qa.md` — a topic's `qa.md` inherits the same tags as its `notes.md`.

## Recording Q&A (on request)

When asked to record a question and answer, append it to the topic's `qa.md` (create the file if it doesn't exist yet) in this format:

```markdown
# <Topic title>

## <Question, as asked (lightly cleaned up for clarity if needed)>

<Answer>

## <Next question>

<Answer>
```

One H1 per file — the topic title, set once when the file is first created. One H2 per question, holding the question itself. Plain prose underneath each, as the answer. Append new Q&A pairs at the end, in the order they were asked — don't reorder or merge existing entries.

## Conventions within a topic

- `topics/<slug>/notes.md` — the structured concept write-up.
- `topics/<slug>/qa.md` — recorded Q&A, created/updated only on explicit request (see above).
- `topics/<slug>/labs/` — hands-on practice, if this topic has any and `subject` calls for it. Name and shape this however actually fits `subject` — it won't always mean "labs," or code at all. When `subject` does involve writing code, this repo's convention is:
  - a single-exercise topic keeps `instructions.md` + source + `Makefile` (or equivalent build/run instructions) directly in `labs/`
  - a multi-exercise topic (e.g. `processes`, with five separate labs) nests numbered subfolders under `labs/` (`labs/01-pid-info/`, `labs/02-fork-basics/`, ...), each with its own `instructions.md` + source + `Makefile`
  - source files are **skeletons with `TODO` markers** — don't hand over a finished solution; filling the gaps is the point of a lab
  - Exception: a lab about *using a tool* (a debugger, a tracer, a profiler, ...) rather than *writing code* can be fully working/ready-to-run, since the exercise is driving the tool, not filling in gaps.
- Read a topic's `notes.md` first, then work through any hands-on practice in order — later labs in a topic build on earlier ones.
- Language/tooling choices are documented in whichever track plan(s) reference a topic — check there rather than assuming.

## Verifying before claiming something works

- Before describing something as working, actually verify it in a way appropriate to `subject` — when `subject` involves code, that means compiling/running it, not just describing what should happen. Warnings on unfilled `TODO`s (unused variables, etc.) are expected and fine; real errors are not.
- Clean up build artifacts after verifying, if `subject` involves any.
- Run `git status` before any broad change, and before anything that could discard uncommitted work.
