# Kernel internals: how you'd build it yourself (theory)

Tags: #os #kernel #interrupts #memory #scheduling

**Bonus, theory-only module** — a fork off the `operating-systems` track's main sequence, not a continuation of it. The [CPU architecture](../cpu-architecture/notes.md) module (interrupts) and the [virtual memory](../virtual-memory/notes.md) module explained interrupts, paging, and syscalls from the *outside* — what a process sees, running on a kernel someone else built. This module explains the same mechanisms plus scheduling from the *inside*. No lab yet — real practice here needs either a minimal bare-metal kernel in an emulator or Linux kernel modules; revisit if that becomes worth it.

## 1. GDT, IDT, and exception handling

**The GDT**, even in 64-bit mode where segmentation is mostly turned off, still needs a handful of required entries:

```
┌───────┬────────────────────────────┐
│ Index │ Entry                      │
├───────┼────────────────────────────┤
│   0   │ null descriptor (required) │
│   1   │ kernel code segment        │
│   2   │ kernel data segment        │
│   3   │ user code segment          │
│   4   │ user data segment          │
│   5   │ TSS descriptor (16 bytes)  │
└───────┴────────────────────────────┘
       lgdt {limit, base} → CPU
```

**The IDT** is 256 entries, one per vector number, each pointing at a handler plus metadata:

```
┌─────────────────────────────────────────────┐
│ handler address (split across 3 fields)      │
│ segment selector  — which GDT entry to run under
│ DPL               — min. privilege allowed to trigger it deliberately
│ IST index         — force a known-good dedicated stack (double-fault, NMI)
└─────────────────────────────────────────────┘
```
```c
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_entry idt[256];   // fill in the ones you care about, lidt to load
```

Some vectors worth knowing by number:

| Vector | Meaning | Has error code? |
|---|---|---|
| 0 | Divide by zero | no |
| 6 | Invalid opcode | no |
| 8 | Double fault | yes (always 0) |
| 13 | General protection fault | yes |
| 14 | Page fault | yes — see §2 |
| 32+ | Hardware IRQs (timer, keyboard, ...), remapped here | no |

**Why a handler is a stub, not your C function directly** — the CPU only auto-saves a fixed set of things on entry (an error code, for some vectors; then `rip`/`cs`/`rflags`/`rsp`/`ss`); it never touches your general-purpose registers. So:

```
 user/kernel code running
        │  interrupt/exception fires
        ▼
 CPU pushes: [error code] rip cs rflags rsp ss
        ▼
 jump to IDT[vector]'s handler — a small ASM stub
        ▼
 stub pushes rax, rbx, rcx, ... (everything the CPU didn't)
        ▼
 call common_c_handler(frame)
        ▼
 stub pops those registers back
        ▼
 iretq   — restores rip/cs/rflags/rsp/ss, resumes at the prior privilege level
```

This is the same save/restore idea as the [CPU architecture](../cpu-architecture/notes.md) module's stack section — just triggered by hardware instead of a `call`/`ret` pair, and returned from with `iretq` instead of plain `ret` because it also has to restore the privilege level.

## 2. Building and walking page tables

x86-64 paging is a 4-level tree, rooted at `CR3`:

```
        CR3
         │
         ▼
   ┌───────────┐
   │   PML4    │   512 entries, 9 bits of the address select one
   └─────┬─────┘
         ▼
   ┌───────────┐
   │   PDPT    │   ← or map a 1 GiB page directly here
   └─────┬─────┘
         ▼
   ┌───────────┐
   │    PD     │   ← or map a 2 MiB page directly here
   └─────┬─────┘
         ▼
   ┌───────────┐
   │    PT     │
   └─────┬─────┘
         ▼
   4 KiB physical page + 12-bit offset
```

A virtual address's bits split exactly into the indices used at each level:

```
 47      39 38      30 29      21 20      12 11         0
┌──────────┬──────────┬──────────┬──────────┬────────────┐
│ PML4 idx │ PDPT idx │  PD idx  │  PT idx  │   offset    │
│  9 bits  │  9 bits  │  9 bits  │  9 bits  │  12 bits    │
└──────────┴──────────┴──────────┴──────────┴────────────┘
```

**Worked example** — virtual address `0x0000000000401000` (a typical low executable load address):

| Field | Value |
|---|---|
| PML4 index | `0` |
| PDPT index | `0` |
| PD index | `2` |
| PT index | `1` |
| offset | `0x000` |

So the MMU walks `PML4[0] → PDPT[0] → PD[2] → PT[1]`, and the final entry's physical address, plus the `0x000` offset, is where the actual byte lives.

**Building it yourself:** allocate physical memory for each table (a simple bitmap or free-list frame allocator is enough for a small kernel), fill in entries mapping your kernel's own code/data somewhere sane, load the top table into `CR3` — paging is now yours.

**Your own page-fault handler** (vector 14, built via §1's IDT machinery) reads:
- `CR2` — always holds the address that caused the *most recent* page fault
- the error code pushed on the stack — read/write? user/supervisor?

and decides: fix it and resume (growing a heap, completing a COW copy — the [virtual memory](../virtual-memory/notes.md) module's policy, now as your own code), or kill the task.

The **TSS** ties this to §1: it holds a known-good kernel stack pointer that the CPU switches to automatically on any privilege-level-changing interrupt — without it, a user task's interrupt would run kernel code on the user's own untrusted stack.

## 3. `SYSCALL`/`SYSRET`, from the kernel's side

At boot, a few Model-Specific Registers get set via `wrmsr`:

| MSR | Purpose |
|---|---|
| `IA32_LSTAR` | the address `syscall` jumps to — no table lookup, straight there |
| `IA32_STAR` | which GDT segment selectors to use for `CS`/`SS`, entering and returning |
| `IA32_FMASK` | which `RFLAGS` bits to clear automatically on entry (e.g. interrupts-enabled) |
| `IA32_EFER` (SCE bit) | must be set at all, or `syscall`/`sysret` don't work |

```
 user mode (ring 3)                        kernel mode (ring 0)
 ───────────────────                       ─────────────────────
 rax = syscall number
 rdi,rsi,rdx,r10,r8,r9 = args
 syscall ────────────────────────────▶  jump straight to IA32_LSTAR
                                         rcx = return addr, r11 = old rflags
                                         (rsp NOT switched automatically!)
                                         swapgs → find a real kernel stack
                                         syscall_table[rax](args...)
                                         rax = return value
 (resumes in user mode)  ◀────────────  sysret   (uses IA32_STAR's selectors)
```

The one sharp edge: unlike an IDT-based interrupt using an IST, `syscall` does **not** switch `rsp` for you. Step one, before anything else, is getting onto a real kernel stack — staying on a small, untrusted user stack while running kernel code is exactly the kind of bug that becomes a security hole.

```c
typedef long (*syscall_fn)(long, long, long, long, long, long);

syscall_fn syscall_table[] = {
    [0]  = sys_read,
    [1]  = sys_write,
    [60] = sys_exit,
    // ...
};
```

This is the exact other end of the `raw.s` lab from the [CPU architecture](../cpu-architecture/notes.md) module — that lab's `syscall` instruction jumped into a handler shaped exactly like this one, just one Linux already built.

## 4. Context switching and scheduling policy

**The mechanism** — each task just needs its own stack pointer saved:

```
switch_to(prev, next):
  1. push prev's callee-saved registers onto prev's own current stack
  2. prev->rsp = current rsp        (remember exactly where we left off)
  3. rsp = next->rsp                (jump onto next's stack)
  4. pop next's callee-saved registers back off its stack
  5. ret                            (resumes wherever "next" last called
                                      switch_to — possibly a totally
                                      different function entirely)
```
```
   prev's stack                  next's stack
  ┌─────────────┐               ┌─────────────┐
  │ ...         │               │ saved regs  │  ← next->rsp, saved
  │ saved regs  │ ← prev->rsp   │ ...         │    the last time IT
  └─────────────┘   (just now)  └─────────────┘    called switch_to
```

This routine is genuinely short — 15–20 lines of assembly in most real kernels.

**What triggers a switch:**
```
 cooperative:   task code ──calls──▶ yield() ──▶ switch_to(next)

 preemptive:    timer IRQ ──▶ IDT[32] handler ──▶ scheduler_tick() ──▶ switch_to(next)
                (fires on a fixed schedule, no matter what the current task is doing)
```
Cooperative alone is enough to demonstrate the raw mechanism, and needs none of §1–§3. Preemptive is what makes multitasking robust against buggy or CPU-bound tasks — the concrete mechanism behind the [CPU architecture](../cpu-architecture/notes.md) module's "preemptive multitasking is built on the timer interrupt."

**Scheduling policy** is a separate, layered decision on top — *which* task runs next:

| Policy | Idea | Tradeoff |
|---|---|---|
| Round-robin | fixed queue, always run the next one | simple; fair by task *count*, not by actual need |
| Priority queues | several queues, by priority level | low-priority tasks can starve under load |
| CFS (real Linux) | track each task's "virtual runtime," always run whoever's had the least | fair by actual CPU time; more bookkeeping |

**Where a future Threads module (not yet built) fits:** a pthread is a request to *this same* kernel scheduler to manage one more schedulable entity — Linux treats processes and threads both as "tasks," distinguished mainly by what they share (address space, file descriptors, ...). That module would ask an already-built version of this machinery to do work; this module is how that machinery gets built.
