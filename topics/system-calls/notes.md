# System calls: crossing from user space into the kernel

Tags: #os #kernel #syscalls #cpu #memory

Prerequisites: [CPU architecture](../cpu-architecture/notes.md), [Virtual memory & paging](../virtual-memory/notes.md)

A system call is a controlled request from a user-space program to the kernel. The kernel implementation of `read`, `write`, `openat`, and the other system calls is **not copied into the program's executable**. The executable contains only the user-space instructions that prepare the syscall number and arguments and eventually execute a special CPU instruction such as `syscall`.

A libc function can sit above that boundary:

```
application code
    ↓ calls read()
libc wrapper in user space
    ↓ loads registers and executes syscall
CPU-controlled privilege transition
    ↓
kernel entry code and syscall implementation
```

If a program invokes `syscall` directly, as in the `raw.s` lab, even the libc wrapper is absent. The executable still contains the `syscall` instruction, but not the kernel's implementation.

## What the CPU changes

On x86-64 Linux, the broad sequence is:

1. User-space code loads the syscall number into `rax` and arguments into the syscall ABI registers.
2. The CPU executes `syscall`.
3. Hardware changes the current privilege level from user mode (ring 3) to kernel mode (ring 0).
4. Hardware saves enough user-space state to return later: the following user instruction address goes into `rcx`, and the old flags go into `r11`.
5. Hardware loads the instruction pointer, `rip`, from `IA32_LSTAR`, a CPU register that the kernel configured with the address of its syscall entry routine.
6. The kernel entry code switches to a trusted kernel stack, saves additional registers, validates the syscall number and arguments, and dispatches to the requested kernel implementation.
7. The kernel places the result in `rax` and returns to user mode, normally through `sysret` or another safe return path. Execution resumes at the user-space instruction immediately after `syscall`.

So the intuition that the program counter changes is right. On x86-64, the architectural name is the **instruction pointer**, `rip`, rather than “process counter.” More generally, textbooks call it the **program counter** (PC).

## Does it enter a different address space?

This wording needs one important correction. The CPU begins executing kernel code at a different **virtual address**, but on mainstream Linux it does not normally switch to an entirely separate virtual address space just because a syscall occurred.

A process's page tables generally describe both:

- user-space mappings, accessible while running in user mode; and
- kernel mappings, protected so user-mode instructions cannot access them.

The privilege transition makes those protected kernel pages accessible to the CPU while it executes kernel code. Modern mitigations such as Kernel Page-Table Isolation (KPTI) can cause a page-table switch on entry and return, so the precise implementation may use different page-table views. But “a syscall always switches to another address space” is not the essential rule. The essential transition is:

```
user privilege + user instruction address
              ↓ syscall
kernel privilege + kernel instruction address
```

The currently executing task is still the same process at first. The kernel is servicing that process's request in kernel mode. A scheduler may block it or switch to another task during the syscall, but that is a separate event, not an inherent part of every syscall.

## What belongs to the executable

| Piece | In the program's executable? |
|---|---|
| Application's call to a libc function | Yes |
| Direct `syscall` instruction | Yes, if emitted directly or inside statically linked wrapper code |
| Dynamically linked libc wrapper | Usually in the separately mapped libc shared library |
| Kernel syscall entry routine | No |
| Kernel implementation of the requested operation | No; it belongs to the running kernel |

That is why the same executable can run against different compatible kernel versions: it depends on the stable syscall interface, not on carrying kernel implementation code inside itself.
