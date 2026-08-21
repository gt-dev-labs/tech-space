# 01 — CPU architecture primer

This is a primer, not a full computer-architecture course — just enough to read assembly (like `hello.s` from module 00) with real understanding, and to make later topics (context switches, syscalls, crashes) make sense at the hardware level instead of feeling like magic.

We're on **x86-64** (confirm with `uname -m`) — the instruction set architecture (ISA) most desktop/server Linux boxes run. ARM is the other major one you'll run into (phones, Apple Silicon, and increasingly cloud/k8s nodes) — same general concepts, different actual instructions and calling convention.

## Registers

Registers are small storage slots built directly into the CPU — vastly faster to access than RAM, but there are only a handful of them. x86-64 has 16 general-purpose 64-bit registers: `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`, `rbp`, `rsp`, `r8`–`r15`. A few carry conventional roles you've already seen in `hello.s`:

- **`rip`** — instruction pointer: the address of the *next* instruction to execute. Every instruction implicitly advances it; `jmp`/`call`/`ret` set it directly.
- **`rsp`** — stack pointer: the current top of the stack (see below).
- **`rbp`** — base/frame pointer: conventionally marks the start of the current function's stack frame (`movq %rsp, %rbp` in the `main` prologue you read earlier).

Registers are also addressable at smaller widths — `rax`/`eax`/`ax`/`al` are literally the same physical register, viewed as 64/32/16/8 bits. That's why `movl $0, %eax` (32-bit) was enough to zero your return value, rather than needing `movq` on the full 64-bit `rax`.

There's also a **flags register** (`rflags`) holding condition codes (zero, carry, sign, overflow) set by comparisons and arithmetic, which conditional jumps (`je`, `jne`, `jg`, ...) read — useful to know exists, not worth memorizing right now.

## The stack, mechanically

You already know the stack as "a memory segment that grows down" from module 02. Here's the actual mechanism:

- `rsp` always points at the current top (lowest address in use) of the stack.
- `push` decrements `rsp` by the operand's size, then writes the value there.
- `pop` reads the value at `rsp`, then increments `rsp`.
- `call target` pushes the return address (the current `rip`, i.e. the instruction right after the `call`) onto the stack, then jumps to `target`.
- `ret` pops that saved address off the stack and jumps to it.

That's the entire mechanism behind the prologue/epilogue you saw in `main`:

```asm
pushq %rbp        ; save caller's frame pointer
movq  %rsp, %rbp   ; establish this function's own frame pointer
...
popq  %rbp        ; restore caller's frame pointer
ret               ; pop return address, jump back to caller
```

## Calling convention (the ABI)

Registers alone don't tell you *which* register holds a function's first argument — that's a convention, not a hardware requirement, called an ABI (Application Binary Interface). On Linux x86-64 (System V AMD64 ABI): the first six integer/pointer arguments go in `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` in that order; further arguments spill onto the stack; the return value comes back in `rax`.

You already saw this concretely: `movq %rax, %rdi` right before `call puts@PLT` — that's placing the string's address into the *first-argument register*, because `puts` takes one argument. This convention is exactly what lets code compiled by different compilers (or even different languages) call each other correctly — as long as both sides agree on the ABI, neither needs to know how the other was implemented.

## Reading the instructions themselves

A few categories cover almost everything you'll see for a while:

- **Data movement** — `mov` family (`movq`, `movl`, ...) copies a value between registers/memory
- **Address computation** — `lea` ("load effective address") computes an address without reading memory at it — used for `leaq .LC0(%rip), %rax` to get a string's address, not its contents
- **Arithmetic/logic** — `add`, `sub`, `and`, `or`, `xor`
- **Comparison** — `cmp`, `test` — set flags for a following conditional jump
- **Control flow** — `jmp` (unconditional), `je`/`jne`/`jg`/... (conditional, based on flags), `call`/`ret`

One practical gotcha: gcc emits **AT&T syntax** by default on Linux (`op src, dst`, `%register`, `$immediate` — what you've been reading), but most online tutorials/Intel's own manuals use **Intel syntax** (`op dst, src`, no `%`/`$`). Same instructions, reversed operand order, different punctuation. If a tutorial's assembly looks backwards from what gcc gave you, that's why. `objdump -d -M intel <binary>` shows Intel syntax if you want it.

## The fetch-decode-execute cycle (conceptually)

At a high level, a CPU core just loops: read the instruction at `rip`, figure out what it means and what it operates on, execute it (read/write registers or memory as needed), then move `rip` to the next instruction (or somewhere else entirely, for jumps/calls/returns) — repeat, billions of times a second. Real CPUs pipeline and reorder this heavily for speed; that's a rabbit hole we're deliberately not going down here.

## The real entry point: `_start`, and talking to the kernel directly

Every `.s` file you've looked at so far had a `main` label, and got linked with plain `gcc`. That linking step silently pulls in the C runtime startup files (`Scrt1.o` and friends), which define the *actual* ELF entry point — a symbol called **`_start`** — and its job is to do libc setup (argv/envp, `atexit` handlers, etc.) and only then call your `main`, and afterward call the real `exit()` syscall for you. `main` was never the true starting point; it's just where libc's own startup code happens to jump to.

You can skip all of that and write `_start` yourself, with zero libc involved. Doing this also exposes the layer beneath *everything* you've been calling so far — even `puts`, at the very bottom, just wraps a **syscall**: a direct request to the kernel. The syscall calling convention on x86-64 Linux is almost the same as the regular ABI, with one difference: arguments go in `rdi`, `rsi`, `rdx`, **`r10`**, `r8`, `r9` (not `rcx` for the 4th argument — the `syscall` instruction itself clobbers `rcx` to hold the return address, so it can't also be used for an argument), and the syscall number itself goes in `rax`. The `syscall` instruction then traps into the kernel directly. Two syscalls, `write` (number 1) and `exit` (number 60), are enough to write a message and quit — no `main`, no libc, no PLT/GOT (there's nothing dynamic to resolve if you never call a shared library), assembled and linked directly with `as`/`ld` instead of `gcc`. See the lab for the hands-on version.

## Interrupts, traps, and privilege levels

An **interrupt** is an event that makes the CPU stop the instruction stream it's currently running and jump to a fixed handler, then usually resume afterward. Two flavors:

- **Hardware interrupts (asynchronous)** — raised by a device (timer, network card, disk, keyboard) at a moment that has nothing to do with what the CPU happens to be executing. A timer interrupt in particular fires on a fixed schedule no matter what code is running.
- **Software traps / exceptions (synchronous)** — raised by the instruction the CPU is executing right now: either deliberately (the `syscall` instruction from the previous section, or the older `int 0x80`), or because the CPU hit a condition it can't proceed past (divide-by-zero, invalid opcode, or a **page fault** — which module 03 already covered without using this word: a page fault genuinely is a CPU exception, and the kernel's fault handler is genuinely an exception handler).

The CPU finds the right handler via the **IDT** (Interrupt Descriptor Table) — a table the kernel sets up at boot, indexed by a vector number (0–255), each slot pointing to a specific handler address. Any interrupt/trap/exception looks up its vector, jumps to that handler, and (for most cases) returns to exactly where it left off once the handler is done.

**Privilege levels (rings):** the CPU distinguishes user code (ring 3 — restricted: can't touch hardware directly, can't modify page tables, can't disable interrupts) from kernel code (ring 0 — unrestricted). Crossing from ring 3 to ring 0 is *exactly* what happens on any interrupt, trap, or `syscall` — it's the hardware-enforced boundary that makes an OS able to enforce anything at all (memory protection, permission checks, resource limits). Without it, a user process could just do whatever it wanted, no different from the kernel.

This reframes `syscall` from the previous section precisely: it's a deliberate, fast software trap into ring 0, using a dedicated fast path (special registers the kernel configures at boot) rather than the slower general interrupt machinery — but it's the same fundamental event as any other interrupt: stop, save enough state to resume later, jump to a kernel handler, eventually return.

**Why this matters beyond terminology:**

- **Preemptive multitasking is built on the timer interrupt.** A process never has to cooperate for the kernel to regain control — the periodic timer interrupt traps into the kernel regardless of what the process is doing, and *that's* the actual mechanism behind a Running→Ready transition (module 02) and a context switch (module 01, registers section): interrupt fires → kernel's scheduler runs → it saves the current process's registers and loads a different process's → returns from the interrupt into different code than where it was raised. Without this, a process stuck in an infinite loop with no syscalls could hang a whole CPU core forever.
- **I/O completion is signaled by hardware interrupts too.** A network card raises an interrupt when a packet arrives; the kernel's handler does minimal work (mark the data ready, wake up whoever's waiting on that file descriptor) and returns fast; the waiting process's blocked `read()`/`epoll_wait()` call actually resumes later. This is the real mechanism behind why `epoll_wait()` can sleep efficiently and still wake up exactly when data shows up — worth remembering once we get to I/O multiplexing and nginx/Node's event loops.

## Why this matters going forward

- **Debugging** — `gdb` shows you exactly this: register values, one instruction at a time. The lab below does this on `hello`.
- **Crashes** — a segfault's crash address and stack trace are just `rip` and a walk up the stack using `rbp` chains — no longer opaque once you've seen this mechanism directly.
- **Context switches** (module 05, threads) — a context switch is fundamentally "save this thread's registers somewhere, load a different thread's registers" — now you know concretely what's being saved and restored.
