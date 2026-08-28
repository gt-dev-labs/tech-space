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

## Why kernel mode is necessary

Kernel code must perform operations that ordinary application code is forbidden to perform. Examples include:

- accessing device-control registers and issuing hardware I/O;
- changing page tables and memory permissions;
- configuring interrupt handling;
- scheduling tasks and changing their saved CPU state;
- accessing kernel memory and other processes' protected resources.

The CPU enforces this distinction. In user mode, privileged instructions are rejected and protected kernel pages cannot be accessed. In kernel mode, the CPU permits those operations.

This is also what makes the kernel a trustworthy gatekeeper. A process cannot open any file merely because it knows the filename or read another process's memory merely because it knows an address. It must ask the kernel, and kernel code checks the caller's identity, permissions, resource limits, and arguments before performing the operation.

If the CPU stayed in user mode, one of two designs would result:

1. **The syscall implementation would remain restricted.** It could not manipulate page tables, devices, interrupt state, or protected kernel data, so it could not actually implement many operating-system services.
2. **User mode would be given those powers.** Then every application would also have them. A buggy or malicious program could overwrite the kernel, inspect other processes, disable interrupts, corrupt storage, or take over the machine.

The privilege transition is therefore not needed merely because the implementation lives at another address. A normal function call can jump to another address without changing privilege. It is needed because the kernel routine must execute with powers that the caller deliberately does not have.

The safe boundary is:

```
untrusted request in user mode
          ↓ syscall
trusted validation and operation in kernel mode
          ↓ return
restricted user mode again
```

## What if user code jumps directly to a kernel address?

Knowing a kernel virtual address does not grant access to it. On every instruction fetch, the CPU's MMU translates the virtual address through the current page tables and checks the page's permission bits against the CPU's current privilege level.

On x86-64, a page-table entry has a **User/Supervisor** permission bit:

- a user page may be accessed while the CPU is running at user privilege;
- a supervisor page may be accessed only while the CPU is running at kernel privilege.

A simplified x86-64 leaf page-table entry looks like this:

```
63             51                    12 11       3  2   1   0
┌───────────────┬──────────────────────┬───────────┬───┬───┬───┐
│ NX            │ physical frame       │ other     │U/S│R/W│ P │
└───────────────┴──────────────────────┴───────────┴───┴───┴───┘
```

- `P` (bit 0): page is present.
- `R/W` (bit 1): writes are allowed when set.
- `U/S` (bit 2): user-mode access is allowed when set; clear means supervisor-only.
- `NX` (bit 63): instruction execution is forbidden when set.

Suppose the physical page frame begins at `0x0000000012345000`. Ignoring hardware-maintained and caching flags, three example entries are:

| Mapping | Simplified entry | Important bits |
|---|---:|---|
| Kernel code: present, read-only, executable, supervisor-only | `0x0000000012345001` | `P=1, R/W=0, U/S=0, NX=0` |
| User code: present, read-only, executable | `0x0000000012345005` | `P=1, R/W=0, U/S=1, NX=0` |
| User data: present, writable, non-executable | `0x8000000012345007` | `P=1, R/W=1, U/S=1, NX=1` |

These examples show that the same physical frame address could theoretically be described with different permissions; the flags are part of the mapping. Real x86-64 translation walks several levels, and effective permissions are combined across them, but the final access decision has this same shape.

Kernel code pages are supervisor pages. If a process in user mode puts a kernel address into `rip` using `jmp`, `call`, or `ret`, the next instruction fetch requires all of the following: a present translation, permission for user-mode access, and permission to execute. A kernel-code mapping has `U/S=0`, so the MMU rejects the fetch and the CPU raises a **page-fault exception** before the kernel instruction executes.

```
CPL 3: user mode
    ↓ jmp kernel_address
MMU translates kernel_address
    ↓ page is supervisor-only
permission check fails
    ↓ #PF page-fault exception
kernel handles the fault
    ↓ usually delivers SIGSEGV to the process
```

With Kernel Page-Table Isolation, most kernel pages may not be present in the user-mode page-table view at all. The result is still a page fault: the translation is missing rather than present-but-supervisor-only.

Suppose the process instead copies the same machine-code bytes into one of its own executable user pages. Those bytes can be fetched, but the CPU remains in user mode. Any instruction defined as privileged—such as changing page-table control registers or disabling interrupts—causes a **general-protection exception**. Any attempt to access supervisor-only kernel memory causes a page fault. Copying or reaching the instructions therefore does not copy the authority required to execute their privileged effects.

The two protections are separate:

| Attempt | Hardware check | Typical CPU exception |
|---|---|---|
| Fetch instructions from a kernel page while in user mode | Page-table User/Supervisor and execute permissions | Page fault (`#PF`) |
| Execute a privileged instruction from an accessible user page | Current privilege level required by that instruction | General protection fault (`#GP`) |
| Read or write protected kernel data | Page-table User/Supervisor permissions | Page fault (`#PF`) |

The kernel's page-fault or general-protection handler receives control in kernel mode. For an invalid user-space attempt, Linux normally turns the fault into a signal such as `SIGSEGV`, which terminates the process unless it has an applicable signal handler.

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
