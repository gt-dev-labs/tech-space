# Learning Systems

A personal curriculum for learning systems programming, OS internals, and networking. Notes (конспект) plus hands-on labs in C, working toward understanding how nginx and Node.js work under the hood, and how Docker, Kubernetes, and OS networking fit together.

If you're an AI agent working in this repo, read [`AGENTS.md`](AGENTS.md) first — it covers communication conventions, curriculum depth expectations, and repo structure.

## Structure

- `notes/` — one folder per module:
  - `notes.md` — the structured concept write-up (конспект)
  - `qa.md` — where a module involved a real back-and-forth (not just reading the note), a record of what came out of that conversation: the actual questions asked, the wrong turns and corrections along the way, and the commands/output that settled things. Not every module has one — only add it when a genuine discussion happened.
- `labs/` — hands-on C (or assembly) exercises, one directory per module; each lab has an `instructions.md`, one or more source skeletons with `TODO` markers, and a `Makefile`

## Roadmap

- [x] 00 — Toolchain: compiling and running C by hand
- [ ] 01 — CPU architecture primer
- [x] 02 — Processes
- [ ] 03 — Virtual memory & paging
- [ ] 04 — System calls
- [ ] 05 — Threads
- [ ] 06 — IPC
- [ ] 07 — I/O multiplexing (select/poll/epoll)
- [ ] 08 — Networking & sockets
- [ ] 09 — Case studies: nginx & Node.js/libuv event loops
- [ ] 10 — Namespaces & cgroups
- [ ] 11 — Docker & Kubernetes architecture
- [ ] 12 — OS networking deep dive

Modules 01 and 03 were inserted after starting 00/02, prompted by questions that came up while reading `hello.s` — CPU architecture (registers, calling conventions) directly supports reading assembly, and virtual memory/paging deepens the process address-space material and explains fork's copy-on-write for real.

## How to work through a module

Read `notes/<module>/notes.md` first for the concept, then work through that module's labs in order — each lab applies what the note just covered, and later labs build on earlier ones. Check `notes/<module>/qa.md` too, if it exists — it captures follow-up questions and corrections that the plain note doesn't.

**Why C instead of Rust for the fundamentals:** nginx and Node's runtime layer (V8/libuv) are C/C++, and the goal is to learn the raw OS interface (syscalls, memory layout, POSIX) directly rather than through an abstraction on top of it. Rust is worth returning to afterward as a second systems language — it lands better once you've felt the problems it solves firsthand.

## Working on a lab

Each lab directory has:
- `instructions.md` — what to implement and what behavior to expect
- one or more `.c` files with a skeleton and `TODO` markers
- a `Makefile` — `make` to build, `make run` to build and run, `make clean` to remove build artifacts
