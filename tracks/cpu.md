# CPU

Low-level CPU mechanics — registers, instruction execution, calling conventions, interrupts, and whatever else comes up while reading assembly or debugging at the instruction level. A curated plan through the shared topic pool in `../topics/`, grown conversationally rather than planned upfront.

## Plan

1. [CPU architecture primer](../topics/cpu-architecture/notes.md) — labs: [`topics/cpu-architecture/labs`](../topics/cpu-architecture/labs) — registers, the stack mechanism, the calling convention/ABI, instruction categories, interrupts/traps/privilege levels
2. [System calls](../topics/system-calls/notes.md) — how the `syscall` instruction changes privilege and redirects `rip` into a kernel entry routine, then returns to user code

This track will grow as more CPU-specific topics get pulled out of conversation — e.g. a dedicated note on registers, on a specific instruction set feature, on SIMD, whatever comes up. See [operating-systems](operating-systems.md) for how these mechanisms get applied at the OS level (interrupts driving scheduling, traps implementing syscalls) — topics can be referenced by both tracks, since the same CPU mechanism and OS abstraction are being viewed from different sides.
