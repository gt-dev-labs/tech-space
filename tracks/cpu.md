# CPU

Low-level CPU mechanics — registers, instruction execution, calling conventions, interrupts, and whatever else comes up while reading assembly or debugging at the instruction level. A curated plan through the shared topic pool in `../topics/`, grown conversationally rather than planned upfront.

## Plan

1. [CPU architecture primer](../topics/cpu-architecture/notes.md) — labs: [`topics/cpu-architecture/labs`](../topics/cpu-architecture/labs) — registers, the stack mechanism, the calling convention/ABI, instruction categories, interrupts/traps/privilege levels

This track will grow as more CPU-specific topics get pulled out of conversation — e.g. a dedicated note on registers, on a specific instruction set feature, on SIMD, whatever comes up. See [operating-systems](operating-systems.md) for how these mechanisms get applied at the OS level (interrupts driving scheduling, traps implementing syscalls) — the CPU architecture topic above is referenced by both tracks, since it's genuinely foundational to each in a different way.
