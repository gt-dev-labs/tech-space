# Systems Programming

General programming-craft/build-tooling topics that aren't specific to any one OS or language runtime. A curated plan through the shared topic pool in `../topics/`.

The goal here isn't C-the-language for its own sake — it's using C as a vehicle to learn how to actually build software close to the machine: manual memory management, data layout, debugging when there's no safety net, and the discipline that comes with it. C is a good fit because it doesn't hide any of this behind the runtime the way most other languages do, and a lot of what it forces you to think about (ownership, allocation, undefined behavior, linking) generalizes to systems programming in any language.

## Plan

1. [Toolchain: compiling and running C by hand](../topics/toolchain/notes.md) — labs: [`topics/toolchain/labs`](../topics/toolchain/labs)
2. Build systems & multi-file projects — headers vs. translation units, static and shared libraries, linking, organizing a project across multiple files — *not built yet*
3. Memory management — stack vs. heap, `malloc`/`free`/`realloc`, ownership conventions, and the classic bugs (leaks, double-free, use-after-free) — *not built yet*
4. Data structures & generic techniques in C — structs and pointers, linked lists, dynamic arrays, hash tables, plus the tools C gives you for generic code (`void*`, function pointers, tagged unions, macros) — *not built yet*
5. Debugging & sanitizers — gdb workflows, Valgrind, AddressSanitizer/UBSan, reading a core dump — *not built yet*
6. Testing in C — assertions, a minimal test harness, what "unit test" even means without a language-level test runner — *not built yet*
7. Concurrency in practice — pthreads, mutexes/condvars, and the real bugs (races, deadlocks); builds on the [operating-systems](operating-systems.md) track's Threads topic once that exists, applying it at the code level rather than the kernel-mechanism level — *not built yet*
8. Performance & profiling — `perf`, benchmarking methodology, cache effects and memory layout, when the algorithm isn't the bottleneck — *not built yet*
9. Capstone project — a real, sizeable program (candidate: a small in-memory key-value store with a network front end) that pulls together everything above, plus the [operating-systems](operating-systems.md) track's process/networking/multiplexing topics once they exist — *not built yet*

## Notes on this plan

- OS-internals content (processes, virtual memory, syscalls, threads-as-a-kernel-mechanism, networking, IPC, ...) lives in the [operating-systems](operating-systems.md) plan instead — this track only handles the C-craft layer on top: how to write, structure, debug, and harden the code itself.
- Steps 2–8 are deliberately building blocks toward step 9: each one is both a standalone topic and a piece the capstone project will actually use. Order can shift if a step turns out to depend on more than expected once it's underway.
- Step 7 is a case of a genuinely cross-cutting topic (see `AGENTS.md`): the underlying mechanism (kernel scheduling of threads, `clone()`) belongs to the operating-systems track, while the correctness/craft side (writing code that's actually thread-safe) belongs here.
