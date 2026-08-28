# Learning Tech

A general-purpose, conversational learning framework, built for one person at a time — see [`config.json`](config.json) (`native_language`, `working_language`, `subject`) for what's currently configured. Notes (конспект) plus hands-on practice where the subject calls for it, written and revised as topics come up in discussion rather than from a syllabus decided up front. Notes evolve alongside the conversation that produced them; a topic's Q&A log (`qa.md`) is recorded separately, only on explicit request. Content lives as a **flat, shared pool of topics** — each one a permanent, numberless slug in `topics/<slug>/`, holding its `notes.md`, optional `qa.md`, and any hands-on practice side by side — with **tracks** as curated reading plans on top, each just a `tracks/<name>.md` file that links to topics from the shared pool in whatever order that domain needs. A topic doesn't belong to one track; any number of tracks can reference the same topic as a step in their own plan, and tracks evolve — reordered, extended, or created from scratch — as new topics get written down.

If you're an AI agent working in this repo, read [`AGENTS.md`](AGENTS.md) and [`config.json`](config.json) first — they cover communication conventions, the conversation-first/on-request workflow, the current subject/languages, and how topics/tracks/tags fit together.

## Tracks

| Track | Covers |
|---|---|
| [`operating-systems`](tracks/operating-systems.md) | OS internals and networking — how Linux actually works, from userspace up |
| [`cpu`](tracks/cpu.md) | Low-level CPU mechanics — registers, instruction execution, calling conventions, interrupts |
| [`systems-programming`](tracks/systems-programming.md) | Programming craft for systems software — memory, data structures, debugging, concurrency, and performance in C, building toward a capstone project |
| [`nodejs`](tracks/nodejs.md) | Node.js/libuv event loop internals, with nginx's event-loop model as a comparison example |
| [`docker-kubernetes`](tracks/docker-kubernetes.md) | Container and orchestration architecture — how Docker and Kubernetes actually work |

More tracks get added here as they start — each one is just a new `tracks/<name>.md` file and a row in this table.

## Profile

[`profile/`](profile/) is a running model of the user, not the subject matter — knowledge gaps and recommendations ([`gaps.md`](profile/gaps.md)), things they're already strong at ([`strengths.md`](profile/strengths.md)), and working-language notes ([`english.md`](profile/english.md), named for the language currently set in `config.json`): recurring mistakes worth being conscious of, vocabulary worth diversifying, and useful/idiomatic phrasing worth having active. Maintained the same way `notes.md` is — updated as things become apparent in conversation, not gated on being asked.

## Why a flat pool + track plans, instead of tracks owning their own content

This repo has changed shape a few times already. First it was one flat sequence; that broke once topics stopped being genuinely sequential. Then each domain got its own track folder, owning its own numbered `notes/`/`labs/` — but that just moved the problem: a topic that's genuinely cross-cutting (sockets are both "OS" and "networking") still had to pick exactly one owning track, and that ownership decision kept changing, which meant renumbering the whole track every time. Then, briefly, topics had a permanent slug but still split their own notes and labs across two separate top-level trees that had to be kept in sync by name — merged into one `topics/<slug>/` per topic, since there was no real reason for that split either.

Splitting "what a topic is" from "what order a track presents it in" fixes the renumbering problem at the root, and it's also what makes the conversational workflow actually work: a topic gets written down once, as it comes up and gets substantial enough in conversation, with a permanent slug that never moves. Whichever tracks it's relevant to get updated to reference it — one topic, potentially several tracks, no ownership decision required and nothing to renumber. All the actual sequencing — "step 1, step 2, ..." — lives in each track's own `.md` file as an ordered list of links, which is cheap to edit and never forces a file move. Tags (`#networking #os`, right under a topic's title) handle discovery across the whole pool independent of any one track's plan.
