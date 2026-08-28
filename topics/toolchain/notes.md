# Compiler & toolchain fundamentals

Tags: #toolchain #compilers #linking

Before touching processes, threads, or syscalls, it's worth seeing exactly what happens between writing `hello.c` and running `./hello` — otherwise every later lab's `Makefile` is just a black box that happens to work.

## The four stages

`gcc hello.c -o hello` is actually four separate steps glued together:

1. **Preprocessing** — handles everything starting with `#`: `#include` gets textually replaced by the file's contents, `#define` macros get expanded, `#ifdef` blocks get resolved. Output: still C source, just expanded (a `.i` file).
2. **Compilation** — translates the preprocessed C into assembly for your target architecture (e.g. x86-64). Output: human-readable-ish assembly text (a `.s` file).
3. **Assembly** — the assembler turns that `.s` text into actual machine code, packaged as an **object file** (a `.o` file). It's binary now, but it's not runnable yet — it may reference symbols (like `printf`) that live in other files or libraries, unresolved.
4. **Linking** — the linker takes one or more object files plus whatever libraries they need, resolves all the cross-file symbol references, and produces the final executable.

```
hello.c --(preprocess)--> hello.i --(compile)--> hello.s --(assemble)--> hello.o --(link)--> hello
```

You can run each stage by hand with gcc flags: `-E` (stop after preprocessing), `-S` (stop after compiling to assembly), `-c` (stop after assembling, don't link). See the lab for hands-on practice.

## Why separate compilation matters

Real C projects (nginx has hundreds of `.c` files) never recompile everything on every change. Each `.c` file compiles independently into its own `.o`; the linker combines them at the end. That's *why* headers exist: a `.h` file **declares** a function's signature so other `.c` files can call it, while exactly one `.c` file **defines** its actual body. The compiler only needs the declaration to check calls are well-formed; the linker resolves the actual address at link time.

This is also why touching one `.c` file only requires recompiling *that* file and re-linking — not rebuilding the whole project.

## Common gcc flags

- `-Wall -Wextra` — turn on (most) warnings; leave these on always, warnings catch real bugs (uninitialized variables, mismatched types, unused results)
- `-g` — embed debug symbols, needed for `gdb` to map machine code back to your source lines
- `-O0` / `-O2` — optimization level; `-O0` (default) is easier to debug, `-O2` is what you'd ship
- `-c` — compile/assemble only, produce a `.o`, don't link
- `-o <name>` — name the output file
- `-I<dir>` — add a directory to search for `#include`d headers
- `-l<name>` — link against a library, e.g. `-lpthread`, `-lm` (math)

## Static vs. dynamic linking (just enough to recognize it later)

- **Static linking** copies a library's code directly into your executable.
- **Dynamic linking** (the default for system libraries like libc) just records that your executable *needs* `libc.so` at runtime; the loader maps it in when the program starts. Run `ldd ./some_binary` to see a dynamically linked executable's shared library dependencies — you'll use this again when looking at how nginx/Node binaries are built.

## From linked file to running process

The four stages above end with a linked executable sitting on disk — but that file is a structured container, not just a stream of CPU instructions. Alongside the actual instruction bytes (`.text`), it holds the data those instructions reference (`.rodata`, `.data`, `.bss`), a **symbol table** and **relocations** (bookkeeping for names the linker/loader still has to resolve — see the PLT/GOT trail in [qa.md](qa.md)), and — the part that matters here — an ELF header plus a table of **program headers**, each one a direct instruction to the kernel's loader: "take these bytes from this file offset, and map them at this virtual address, with these permissions."

`readelf -l` on a linked binary shows exactly that table:

```
Type: DYN (Position-Independent Executable file)

  LOAD  Offset 0x0000  VirtAddr 0x0000  ...  R    <- .rodata
  LOAD  Offset 0x1000  VirtAddr 0x1000  ...  R E  <- .text (code)
  LOAD  Offset 0x2db8  VirtAddr 0x3db8  ...  RW   <- .data/.bss/GOT
```

This is the *source* of the process memory layout diagrammed in the [Processes](../processes/notes.md) module — that text/rodata/data/bss/heap/stack picture isn't asserted by the OS, it's built by the loader reading rows exactly like these and mapping each one. The `R E` (readable + executable) vs `R`/`RW` permission bits are what the [virtual memory](../virtual-memory/notes.md) module's page-permission enforcement actually protects — a page without the executable bit can be read or written, but the CPU will fault if anything ever tries to jump into it and run it as code.

One more piece only makes sense once you've seen this table: `Type: DYN` means this is a **PIE** (position-independent executable) — every `VirtAddr` here is actually an *offset*, and the kernel picks a random base to add to all of them on every `exec()` (**ASLR** — see the virtual memory module for the proof). That's why the same binary's `main` can disassemble to a different absolute address on every run, even though the file on disk never changes.

## What `make` is actually doing

Every lab so far handed you a working `Makefile`. Here's what's in one:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -g

hello: hello.c
	$(CC) $(CFLAGS) -o hello hello.c

run: hello
	./hello

clean:
	rm -f hello

.PHONY: run clean
```

- Each `target: dependencies` line, followed by an indented (must be a literal **tab**, not spaces) command, is a **rule**: "to build `target`, make sure `dependencies` are up to date first, then run this command."
- `make` only re-runs a rule's command if the target is missing, or older than one of its dependencies (compares file modification times). That's the entire point — avoiding a full rebuild every time you change one file.
- `CC` and `CFLAGS` are just variables, referenced with `$(...)`.
- `.PHONY: run clean` tells make that `run` and `clean` aren't real files to check timestamps on — always run their commands when asked.
- Running plain `make` builds the *first* target in the file by default.

## Optional: reading the generated assembly

`hello.s` is smaller than `hello.i` even though it comes later in the pipeline. Preprocessing dumps in *everything* a header might offer, as text, unconditionally; compiling only emits code for what your program actually does. A declaration like `extern int printf(...)` produces zero instructions on its own — only your real statements do.

The file has two kinds of lines: **real instructions** (CPU opcodes — `pushq`, `movq`, `call`, `ret`, ...) and **directives** (anything starting with `.` — instructions *to the assembler*, not the CPU). But directives themselves split into two very different groups:

- **Data-emitting directives** — `.string`, `.byte`, `.long`, `.quad`, `.ascii`, `.zero`, and section selectors (`.text`, `.data`, `.section ...`) — tell the assembler "write these literal bytes into the object file, right here." These produce real content that ends up mapped into the running program's memory exactly as written — `.string "Hello, systems!"` is why those exact bytes exist in `.rodata` at all; it's not metadata *about* the program, it *is* program data.
- **Pure-metadata directives** — `.cfi_*`, `.size`, `.ident`, `.file`, `.type`, `.note.*` — produce nothing your code ever reads as data; at most they add debug/note info for tools (`gdb`, the loader's security checks).

Don't let "starts with a dot" read as "harmless to delete" — whether a directive is safe to remove depends on which of these two groups it's in.

Real instructions for `hello.c`'s `main`:

```asm
main:
.LFB0:
	endbr64
	pushq	%rbp
	movq	%rsp, %rbp
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movl	$0, %eax
	popq	%rbp
	ret
```

The `main:` label is where execution starts. Reading it:

- `endbr64` — CPU security marker (Intel CET), gcc inserts this automatically at the start of any indirectly-callable function; ignorable for now.
- `pushq %rbp` / `movq %rsp, %rbp` — function prologue: save the caller's frame pointer, set up this function's own stack frame — the same **stack** segment from the process memory layout above.
- `leaq .LC0(%rip), %rax` — compute the address of the string `"Hello, systems!"` (defined as `.LC0` below, in `.rodata`) into register `rax`. `leaq` computes an address without dereferencing it; `%rip`-relative addressing is how position-independent binaries reference their own data without hardcoded absolute addresses.
- `movq %rax, %rdi` — move that address into `rdi`, the first-argument register per the x86-64 Linux calling convention.
- `call puts@PLT` — gcc silently rewrote `printf("...\n")` with no format specifiers into a faster `puts` call. `@PLT` is the dynamic-linking indirection: `puts` lives in `libc.so`, so this jumps through a PLT stub that the loader patches at program startup (see the static-vs-dynamic-linking section above).
- `movl $0, %eax` — the return value (`eax`) is set to 0 — this is `return 0;`.
- `popq %rbp` / `ret` — epilogue: restore the caller's frame pointer, return.

The instructions themselves, as a reference table (general categories are in the [CPU architecture](../cpu-architecture/notes.md) module; this is specifically the ones that appear in `hello.s`):

| Instruction | Meaning |
|---|---|
| `endbr64` | CPU security marker (Intel CET) — marks a valid landing pad for an indirect call; gcc inserts it automatically, safe to ignore for now |
| `pushq %rbp` | push: decrement `rsp`, then write `%rbp`'s value there — saves the caller's frame pointer onto the stack |
| `movq %rsp, %rbp` | copy `rsp`'s value into `rbp` — establishes this function's own frame pointer |
| `leaq .LC0(%rip), %rax` | "load effective address" — compute an address (here, `.LC0` relative to the current instruction) *without* reading memory at it, and store that address in `rax` |
| `movq %rax, %rdi` | the general `mov` family: copy a value between registers/memory. The `q`/`l`/`w`/`b` suffix picks the width (64/32/16/8-bit) — here, copying the computed address into the first-argument register |
| `call puts@PLT` | push the return address (the instruction right after this `call`) onto the stack, then jump to the target |
| `movl $0, %eax` | `mov`, 32-bit width, with a literal (`$0`) as the source — this is `return 0;` landing in the return-value register |
| `popq %rbp` | pop: read the value at `rsp`, store it in `%rbp`, then increment `rsp` — restores the caller's saved frame pointer |
| `ret` | pop the saved return address off the stack, jump to it |

Common directives, decoded:

| Directive | Meaning |
|---|---|
| `.file "hello.c"` | debug metadata: source file this came from |
| `.text` / `.section .rodata` | "everything after this goes in the **text**/**read-only data** segment" — the same segments from the process memory diagram |
| `.LC0:` | compiler-generated label marking a string literal's bytes, so code can reference its address symbolically before the real numeric address is known |
| `.string "..."` | emit these literal bytes (plus a null terminator) here |
| `.globl main` | export the `main` symbol so other code (the C runtime startup routine that calls `main`) can find it — the same symbol-visibility mechanism linking relies on |
| `.type main, @function` | tells the assembler/linker this symbol is a function, not data |
| `.LFB0:` / `.LFE0:` | function begin/end markers, used to build debug info |
| `.cfi_startproc` … `.cfi_endproc` | Call Frame Information — describes how to unwind the stack at each point (find the caller's saved registers/return address); this is what makes debugger backtraces work. Inert at runtime. |
| `.size`, `.ident` | bookkeeping: symbol size, compiler version string — informational only |
| `.note.GNU-stack`, `.note.gnu.property` | security-hardening flags (e.g. non-executable stack, CET support) read by the kernel/loader when loading the binary |

## Try it yourself

Do the lab before moving on — running the four stages by hand, then building a small multi-file project by hand without `make`, then writing your own `Makefile` for it. After that, go back and re-read the `Makefile`s in `../processes/labs/*` — they should read as plain and boring now.
