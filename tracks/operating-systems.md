# Operating Systems

Understanding how Linux actually works, from userspace up. This is a curated plan through the shared topic pool in `../topics/` — it doesn't own any content itself, it just orders it. Grows conversationally: topics get created on request as questions come up in discussion, then slotted in here wherever they fit, reordering as needed.

## Plan

1. [CPU architecture primer](../topics/cpu-architecture/notes.md) — labs: [`topics/cpu-architecture/labs`](../topics/cpu-architecture/labs) — also see the [CPU](cpu.md) track
2. [Processes](../topics/processes/notes.md) — labs: [`topics/processes/labs`](../topics/processes/labs)
3. [Virtual memory & paging](../topics/virtual-memory/notes.md) — labs: [`topics/virtual-memory/labs`](../topics/virtual-memory/labs)
4. [System calls](../topics/system-calls/notes.md) — the user/kernel boundary, syscall entry, privilege transitions, and what is actually part of an executable
5. Threads — *not built yet*
6. IPC — *not built yet*
7. I/O multiplexing (select/poll/epoll) — *not built yet*
8. Networking & sockets — *not built yet*
9. Namespaces & cgroups — *not built yet*
10. OS networking deep dive — *not built yet*

Steps 1 and 3 were inserted after starting on processes, prompted by questions that came up while reading assembly output — CPU architecture (registers, calling conventions) directly supports reading assembly, and virtual memory/paging deepens the process address-space material and explains fork's copy-on-write for real. The system-calls topic then connects those CPU and memory mechanisms to the actual user/kernel transition.

### Bonus branch — build it yourself (theory only)

- [Kernel internals](../topics/kernel-internals/notes.md) — how you'd build your own GDT/IDT + exception handling, page tables, syscall dispatch, and a scheduler, from a kernel author's side rather than a userspace process's. A fork off the main plan above, not a continuation of it — theory only, since real practice would need a bare-metal kernel or Linux kernel modules.

## Notes on this plan

- **Why C instead of Rust for the fundamentals:** nginx and Node's runtime layer (V8/libuv) are C/C++, and the goal is to learn the raw OS interface (syscalls, memory layout, POSIX) directly rather than through an abstraction on top of it.
- **Toolchain on the dev machine:** `gcc`, `make`, `gdb`, and `strace` are all installed. General C-toolchain fundamentals (compiling/linking by hand, Makefiles) are their own topic — see the [systems-programming](systems-programming.md) plan.
- Read each step's note first, then work through its labs in order. Check the topic's `qa.md` too, if it has one — it captures follow-up questions and corrections the plain note doesn't.
