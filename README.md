# Learning Tech

A personal, multi-domain technical learning repo: notes (конспект) plus hands-on labs, organized as a **flat, shared pool of modules** — each one a permanent, numberless slug in `notes/<slug>/` and `labs/<slug>/` — with **tracks** as curated reading plans on top, each just a `tracks/<name>.md` file that links to modules from the shared pool in whatever order that domain needs. A module doesn't belong to one track; any number of tracks can reference the same module as a step in their own plan.

If you're an AI agent working in this repo, read [`AGENTS.md`](AGENTS.md) first — it covers communication conventions, depth expectations, and how modules/tracks/tags fit together.

## Tracks

| Track | Covers |
|---|---|
| [`operating-systems`](tracks/operating-systems.md) | OS internals and networking — how Linux actually works, from userspace up |
| [`systems-programming`](tracks/systems-programming.md) | General programming-craft/build-tooling topics not tied to a specific OS or runtime — currently the C toolchain |
| [`nodejs`](tracks/nodejs.md) | Node.js/libuv event loop internals, with nginx's event-loop model as a comparison example |
| [`docker-kubernetes`](tracks/docker-kubernetes.md) | Container and orchestration architecture — how Docker and Kubernetes actually work |

More tracks get added here as they start — each one is just a new `tracks/<name>.md` file and a row in this table.

## Why a flat pool + track plans, instead of tracks owning their own content

This repo has changed shape twice already. First it was one flat sequence; that broke once topics stopped being genuinely sequential. Then each domain got its own track folder, owning its own numbered `notes/`/`labs/` — but that just moved the problem: a module that's genuinely cross-cutting (sockets are both "OS" and "networking") still had to pick exactly one owning track, and that ownership decision kept changing, which meant renumbering the whole track every time.

Splitting "what a module is" from "what order a track presents it in" fixes that at the root. A module gets a permanent slug once and never moves or renumbers again, no matter how many tracks reference it or in what order each one wants it. All the actual sequencing — "step 1, step 2, ..." — lives in a track's own `.md` file as an ordered list of links, which is cheap to edit and never forces a file move. Tags (`#networking #os`, right under a module's title) handle discovery across the whole pool independent of any one track's plan.
