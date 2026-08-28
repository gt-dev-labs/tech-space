# System calls: crossing from user space into the kernel

Tags: #os #kernel #syscalls #cpu #memory #interrupts

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

## Where the implementation lives

During boot, the operating system kernel is loaded from its kernel image on disk into physical memory. The boot process establishes virtual-memory mappings for that kernel code and data. While the machine is running, syscall implementations therefore already exist as ordinary machine instructions in memory, inside protected kernel pages.

A syscall does not load its implementation from disk on every call. Broadly, it redirects execution into code that is already resident and mapped:

```
user code executes syscall
           ↓
fixed kernel syscall-entry routine
           ↓
dispatch through syscall table
           ↓
specific routine such as read or write
```

It is not normally a direct jump from the application to the final `read` implementation. The CPU first enters a common kernel entry point. Kernel code saves state, validates the syscall number and arguments, and uses the syscall number to select the appropriate routine.

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

The CPU begins executing kernel code at a different **virtual address**, but on mainstream Linux it does not conceptually need to switch to an entirely separate virtual address space just because a syscall occurred.

A process's page tables can describe both:

- user-space mappings, accessible while running in user mode; and
- kernel mappings, protected so user-mode instructions cannot access them.

The privilege transition makes those protected kernel pages accessible to the CPU while it executes kernel code. Modern mitigations such as Kernel Page-Table Isolation (KPTI) can cause an actual page-table switch on entry and return, so the precise implementation may use different page-table views. But “a syscall always switches to another address space” is not the essential rule. The essential transition is:

```
user privilege + user instruction address
              ↓ syscall
kernel privilege + kernel instruction address
```

The currently executing task is still the same process at first. The kernel is servicing that process's request in kernel mode. A scheduler may block it or switch to another task during the syscall, but that is a separate event, not an inherent part of every syscall.

## Is a system call an interrupt?

A normal x86-64 `syscall` does **not require a hardware interrupt**. The `syscall` instruction itself synchronously causes the privilege transition and redirects `rip` through a dedicated fast syscall-entry mechanism.

The related events are:

| Event | Cause | Timing | Entry mechanism |
|---|---|---|---|
| System call | Current program deliberately executes `syscall` | Synchronous | Dedicated syscall entry configured through `IA32_LSTAR` |
| CPU exception | Current instruction causes a condition such as a page fault or divide-by-zero | Synchronous | Exception vector/handler |
| Hardware interrupt | External hardware signals the CPU, such as a timer or network device | Asynchronous | Interrupt vector/handler |

Older x86 Linux code could make a system call with `int 0x80`. That instruction uses the CPU's software-interrupt mechanism deliberately, so historical explanations often describe system calls as software interrupts or traps. Modern x86-64 Linux normally uses `syscall`, which has a dedicated entry path.

A hardware interrupt can still happen while a syscall is being handled, provided interrupts are enabled at that point. For example, a timer interrupt may temporarily interrupt the kernel's syscall code, run its interrupt handler, and perhaps lead the scheduler to choose another task. The kernel can later continue the original syscall. But this interrupt is incidental; it is not what caused the syscall.

Some syscalls also start asynchronous hardware work. For example, a `read` may ask a device to fetch data and then block the calling task. Later, the device raises a hardware interrupt announcing completion. That interrupt lets the kernel mark the operation complete and wake the blocked task. In that case:

```
syscall enters kernel
    ↓
kernel starts I/O and blocks task
    ↓
device works independently
    ↓
hardware interrupt reports completion
    ↓
kernel eventually resumes task and finishes syscall
```

The syscall transition and the later device interrupt are two separate events that participate in the same operation.

## What belongs to the executable

| Piece | In the program's executable? |
|---|---|
| Application's call to a libc function | Yes |
| Direct `syscall` instruction | Yes, if emitted directly or inside statically linked wrapper code |
| Dynamically linked libc wrapper | Usually in the separately mapped libc shared library |
| Kernel syscall entry routine | No |
| Kernel implementation of the requested operation | No; it belongs to the running kernel |

That is why the same executable can run against different compatible kernel versions: it depends on the stable syscall interface, not on carrying kernel implementation code inside itself.
