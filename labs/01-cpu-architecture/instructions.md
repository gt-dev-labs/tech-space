# Lab 01 — Watching registers and the stack with gdb

This lab needs `gdb`, which isn't installed yet:

```
sudo apt install -y gdb
```

## Part 1 — Step through `main` instruction by instruction

1. `make hello`
2. `gdb ./hello`
3. Inside gdb:
   - `break main` — set a breakpoint at the start of `main`
   - `run` — starts the program, stops at the breakpoint
   - `disas main` — disassemble `main`. Compare it against `labs/00-toolchain/hello.s` — same logic, possibly a different syntax (gdb may default to AT&T or Intel depending on your setup; `set disassembly-flavor att` forces AT&T to match what you already know)
   - `stepi` (or `si`) — execute exactly one machine instruction and stop. Do this repeatedly, and after each one run `info registers rip rsp rbp rax rdi` — watch `rip` advance, and watch `rdi` stay zero/garbage until the `lea`+`mov` pair actually loads the string's address into it
   - Right after `rdi` gets set (just before the `call puts@plt` line), run `x/s $rdi` — this reads memory at the address `rdi` points to, as a string. You should see `"Hello, systems!"` — the literal bytes from `.rodata`, exactly like the `objdump`/hex dump we did earlier, but now read directly out of your program's live memory
   - `continue` — let it run to completion; you'll see the program's real output (`Hello, systems!`) print, interleaved with gdb

## Part 2 — Where a crash actually happens

1. `make crash`
2. `gdb ./crash`
3. `run` — it'll crash with a segmentation fault
4. `info registers rip` — this is the exact address of the instruction that tried to read through a null pointer
5. `disas` — the current instruction (marked with `=>`) is the one that faulted
6. `bt` (backtrace) — shows the call stack at the moment of the crash; trivial here since there's only `main`, but this is exactly the mechanism (walking `rbp`-linked frames on the stack) behind every crash backtrace you'll ever read, including much deeper ones from real programs

## Verify (Part 1 & 2)

- You watched `rdi` go from garbage to the string's real address, and read the string back out of live memory with `x/s`.
- You located the exact faulting instruction and its address in `crash`, and understand that a "stack trace" is just a walk up the chain of saved `rbp` values on the stack.

## Part 3 — Write it yourself, no libc, no `main`, no PLT/GOT

Every program so far has had a `main` and got linked with `gcc`, which silently pulls in libc's startup code. Complete `raw.s`, which defines the *actual* ELF entry point (`_start`) directly and talks to the kernel with raw syscalls — no libc at all.

Fill in the two `TODO` blocks using the hints in the comments: a `write(1, msg, len)` syscall, then an `exit(0)` syscall. Both just need the right registers set (per the comments) followed by the `syscall` instruction.

Build it *without gcc* — straight from assembly to a linked executable:

```
as raw.s -o raw.o
ld raw.o -o raw
./raw
```

(`make raw` does the same two steps.)

## Verify (Part 3)

- `./raw` prints `Hello from assembly!` and exits with status 0 (`echo $?`).
- `ldd ./raw` should say **"not a dynamic executable"** — confirming there's genuinely no libc, no PLT, no GOT involved; there was nothing dynamic left to resolve.
- `readelf -h raw | grep Entry` — the entry point address should match wherever your `_start` label landed; this is the literal address the kernel jumps to when it runs your program, before anything you'd normally think of as "the start" (like `main`) even exists.

## Part 4 — Watching interrupts and traps from userspace

We can't safely write a real interrupt handler from here — that's kernel-module territory, a much bigger jump with real risk of crashing the machine. What we *can* do is observe real ones happening.

1. `cat /proc/interrupts` — a live table of interrupt counts per CPU, per source. Run `watch -n1 cat /proc/interrupts` (or just run `cat /proc/interrupts` twice a few seconds apart and diff by eye) and find counters that keep climbing on their own — that's hardware interrupts firing continuously regardless of what you're doing. (You're likely on WSL2, a Hyper-V VM, so the device list will show virtualized entries like `HV-PCI-MSIX`/`virtio` rather than bare-metal hardware — the concept is identical, just virtualized.)
2. `strace ./raw` — traces every syscall your `raw` binary makes. You should see almost nothing besides `execve` (strace launching it), your one `write`, and your one `exit` — exactly two traps into the kernel, matching exactly the two syscalls you wrote by hand.
3. `strace ./hello` (the libc version from Part 1) — count the lines: `strace ./hello 2>&1 | wc -l`. On this system it's around 40, versus `raw`'s handful. That gap is entirely libc's startup machinery (`mmap`, `arch_prctl`, opening shared libraries, etc.) — every one of those lines is a real ring-3→ring-0 trap that happened before your program ever got to the one line of code you actually wrote.

## Verify (Part 4)

- You found at least one interrupt counter in `/proc/interrupts` that increases on its own between two reads, with no relation to any specific command you ran.
- You can explain, in one sentence, why `strace ./raw` shows far fewer lines than `strace ./hello` even though both programs print one line of text.
