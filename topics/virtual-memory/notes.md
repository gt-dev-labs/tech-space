# Virtual memory & paging

Tags: #os #memory #paging

Module 01 asserted a few things without explaining the mechanism: that two processes' `0x400000` are different physical bytes, that `fork()`'s copy-on-write "just works," and that `.text` can be "shared between processes running the same binary." This module explains *how*, and it all comes down to one idea: **every memory address your program ever uses is fake.**

## Why virtual memory exists

A running program never touches physical RAM addresses directly. Every address it uses is a **virtual address**, private to that process, translated to a real physical address by the hardware on every single access. Three big payoffs:

1. **Isolation** — a bug that corrupts "address `0x1000`" in one process cannot touch another process's actual memory, because their `0x1000`s are (usually) mapped to entirely different physical locations, or one of them may not be mapped at all.
2. **A clean, contiguous-looking layout** — the text/rodata/data/bss/heap/stack diagram from the [Processes](../processes/notes.md) module can look tidy and contiguous in virtual address space even though the underlying physical RAM backing it is scattered and fragmented.
3. **Overcommit** — a process's virtual address space can be far larger than the physical RAM actually installed, because not all of it needs to be backed by real memory at once (more below).

## Pages, page tables, and the MMU

Memory is managed in fixed-size chunks called **pages** — 4 KiB each on x86-64 by default. Virtual address space is divided into virtual pages; physical RAM is divided into physical page **frames**. The kernel keeps, per process, a **page table**: a mapping from virtual page → physical frame, plus permission bits (readable / writable / executable / present-in-memory-at-all).

When the CPU executes an instruction touching a virtual address, a hardware unit called the **MMU** (memory management unit) looks up that address's page in the *current* process's page table to find the real physical address — transparently, on every access. (A small on-CPU cache called the **TLB** remembers recent translations, since walking the full page table every time would be too slow — worth knowing it exists, not worth going deeper on here.)

This is the actual mechanism behind "two processes' `0x400000` are different physical bytes": they're two different page tables, and there's no rule saying they have to point the same virtual page at the same physical frame (unless something — like shared libraries, below — deliberately arranges that).

Page-level granularity also explains why `.text`/`.rodata`/`.data` have to be *separate* segments: permissions (read-only vs. read+execute vs. read+write) can only be set per-page, so any two regions needing different permissions must live on different pages.

## Page faults — not always an error

When the MMU can't complete a translation, the CPU raises a **page fault**, and the kernel's fault handler decides what to do. There are three very different cases:

- **Minor fault, expected** — the page isn't mapped yet, but the kernel knows exactly what should go there (a fresh zeroed page, a copy for COW — see below, or a page from an already-open file backing an `mmap`). The kernel fixes it up and resumes the instruction, invisible to your program except for a tiny one-time delay. This is **demand paging**: a program doesn't need all its memory physically resident before it can run — pages get faulted in only when actually touched.
- **Major fault, expected but expensive** — the page's data lives on disk (swapped out, or not yet read in from a file), so the kernel has to do real disk I/O before resuming. Visible as "major page faults" in tools like `/usr/bin/time -v`.
- **Unfixable — SIGSEGV** — the address has no valid page table entry at all, or the access violates that page's permissions (e.g. writing to a genuinely read-only page). The kernel can't paper over this, so it delivers `SIGSEGV` to the process instead. This is exactly what happened in `../cpu-architecture/labs/crash.c`: dereferencing address `0` means "translate virtual page 0," which has no valid entry — unfixable, `SIGSEGV`.

## fork()'s copy-on-write, precisely

Module 02 described COW informally: "both processes share the same physical pages... until one writes." Now the mechanism: `fork()` doesn't copy any page contents. It builds the child a new page table pointing at the *exact same physical frames* as the parent's, and marks every writable page in *both* page tables read-only. Both processes run normally as long as they only read.

The instant either one tries to **write** to such a page: CPU raises a page fault → kernel sees this is a COW page with more than one referrer → allocates a fresh physical frame → copies the original page's contents into it → updates *only that process's* page table entry to point at the new private, writable frame → resumes the write. One minor fault, one copy, per page actually written to — never per byte, never for pages that are only ever read.

## Shared libraries, previewed

The flip side of COW: multiple *unrelated* processes running the same shared library (`libc.so`) can have their page tables point at the *same* physical frames for that library's code, marked read+execute, no copying ever needed since nobody writes to code. That's the real mechanism behind "`.text` is shared between processes running the same binary" from the [Processes](../processes/notes.md) module — and it's set up via `mmap()`, a syscall a future System calls module will get to. It's also a big part of why running many nginx worker processes is cheaper than it sounds — they're not each holding a separate physical copy of the nginx binary or libc.

## ASLR — why the same binary loads at a different address each run

Everything above explains *that* addresses are virtual and translated per-process. ASLR (Address Space Layout Randomization) is why they're also **unpredictable**: the kernel picks a fresh random base address for a binary's segments (and its stack, heap, and any shared libraries) on every single `exec()`, even for the exact same file.

This only works because modern binaries are built as **PIE** (Position-Independent Executable) — `readelf -h somebinary` reporting `Type: DYN` rather than `EXEC` means every address baked into the file (in its ELF program headers, in `call`/`jmp` targets) is a relative offset, not an absolute address. The loader picks a random base once at `exec()` time, then every one of those offsets becomes `base + offset` for that run only.

Proved this directly rather than trusting the term: compiled a trivial binary (`sleep(2); return 0;`), ran it three separate times, and grepped its own mapping out of `/proc/<pid>/maps` while it was still alive:

```
run 1: 55aab68a3000
run 2: 6236bb883000
run 3: 5c47323b8000
```

Same file on disk, three unrelated base addresses. Every offset from the ELF program headers (see the [toolchain](../toolchain/notes.md) module) gets added to whichever of these the kernel happened to choose that run.

Why it matters: it's a security mitigation, not a performance or correctness feature. Exploits that rely on jumping to a hardcoded address (e.g. the address of a `libc` function, for a return-oriented-programming chain) can't just hardcode it anymore — they have to first leak an address from the running process, which ASLR makes much harder to guess blind. It composes with the [PLT/GOT mechanism](../toolchain/qa.md) for shared libraries: `puts`'s real address isn't just "wherever `libc.so` lives" but "wherever `libc.so` got randomly based *this run*," which is exactly why that address can only be resolved at load time, into the GOT, never baked into a file ahead of time.

## Swap, briefly

Physical RAM can be oversubscribed. If it fills up, the kernel can evict a page that hasn't been used recently out to disk (swap space), freeing the physical frame; touching that page again later triggers a major fault to bring it back. This becomes directly relevant once we get to container memory limits (see the `docker-kubernetes` track, which builds on a future Namespaces & cgroups module) — cgroups can cap how much physical memory a group of processes may hold resident, and when that limit is hit, the kernel reclaims pages or (if configured) invokes the OOM killer.

## Why this matters for your actual goals

- Docker containers use *exactly* this page-table/page-fault machinery — nothing new. "Containerizing" a process doesn't change its memory model at all; it adds cgroups (accounting/limits on physical pages held resident) and namespaces (an isolated *view* of other resources) on top of an ordinary Linux process. That's a big part of why containers are cheap compared to VMs, which virtualize a whole separate memory-management layer in hardware.
- nginx workers and any multi-process server lean on the shared-library sharing above to make N worker processes far cheaper than N independent full memory copies.
- Node.js/V8's heap, and any `mmap`-based I/O you'll see later, are ultimately just more virtual memory regions on top of this same mechanism.
