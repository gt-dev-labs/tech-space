# 00 — Q&A: PLT, GOT, and the object file's symbol table

`notes.md` has the clean writeup. This file records the messier trail that got there — the actual questions asked while poking at `hello.o`/`hello`, the real commands run, and a couple of corrections along the way.

## "Why is it called an object file? What are the 'objects' in it?"

Two different answers, stacked:

1. **Historical** — in old compiler terminology, "object code"/"object program" just meant "the output of compiling," a subject/object naming from long before object-oriented programming existed as a term. Unrelated to OOP objects.
2. **Literal** — ELF symbol tables classify every name a file defines or references by a real type, and one of those types is literally called `OBJECT`. `hello.o` has none, because `hello.c` has no global variable:
   ```
   Num:  Value  Size Type    Bind   Vis      Ndx Name
   2:    0      0    SECTION LOCAL  DEFAULT  1   .text
   3:    0      0    SECTION LOCAL  DEFAULT  5   .rodata
   4:    0      30   FUNC    GLOBAL DEFAULT  1   main
   5:    0      0    NOTYPE  GLOBAL DEFAULT  UND puts
   ```
   Adding one to a throwaway test file shows it for real:
   ```c
   int counter = 100;
   int main(void) { return counter; }
   ```
   ```
   Value  Size Type    Bind   Vis      Ndx Name
   0      4    OBJECT  GLOBAL DEFAULT  3   counter
   0      16   FUNC    GLOBAL DEFAULT  1   main
   ```
   So "the objects in an object file" are, concretely, the named data items and functions it defines or references — exactly what the linker's job is to resolve.

## What the symbol table actually is

Every compiled file (`.o`, and the final executable too) has, alongside its actual code and data bytes, a separate section called `.symtab` — a directory of every named thing the file **defines** (a function or a global variable it actually contains code/data for) and every named thing it merely **references** but doesn't define itself (like `hello.o` referencing `puts`, which lives elsewhere). It's not extra/optional bookkeeping — it's the exact data structure the linker reads to do its entire job: for every "I reference X but don't have it" entry in one file, find an "I define X" entry somewhere else (another object file, or a library) and wire them together. It's also what lets tools like `gdb` show you `main+0x12` instead of a bare address — the symbol table is where that name-to-address mapping lives.

Structurally it's just a list of entries, one per symbol, each carrying the metadata below — this is `.symtab`'s row shape, and it's binary data, unreadable without a tool.

## Reading the symbol table's columns

`readelf -s hello.o` / `nm hello.o` decode that `.symtab` section into something readable.

`nm`'s one-letter codes: `T` = defined in `.text`, `U` = undefined (referenced, not defined here):
```
0000000000000000 T main
                 U puts
```

`readelf -s`'s fuller columns, decoded:

| Column | Meaning |
|---|---|
| `Num` | just the row index — no meaning beyond that |
| `Value` | the symbol's address. Always `0` for every symbol in an unlinked `.o` file — real addresses don't exist until linking assigns them |
| `Size` | size in bytes (a function's code length, a variable's storage size); `0` when the symbol is undefined here, since this file has no idea how big something it doesn't define is |
| `Type` | what kind of thing this is: `FUNC` (a function), `OBJECT` (a data item — variable), `SECTION` (an entry representing a section itself, not "your" code), `FILE` (a marker naming the source file), `NOTYPE` (no type recorded — typical for an external symbol this file only references, like `puts`) |
| `Bind` | linkage scope: `LOCAL` (only visible inside this file — the linker won't let anything else reference it), `GLOBAL` (exported — visible to the linker when combining with other files) |
| `Vis` | visibility refinement on top of `Bind` — `DEFAULT` (the normal case, what you'll see almost always) means ordinary visibility rules apply; other values (`HIDDEN`, `PROTECTED`) restrict visibility further but aren't relevant to anything we've built so far |
| `Ndx` | which section (by number, matching `readelf -S`'s listing) this symbol lives in — or `UND` if it's referenced but not defined anywhere in this file, or `ABS` for a value not tied to any section |
| `Name` | the symbol's name |

`main` is `FUNC`/`GLOBAL`/`Ndx 1` (defined here, in section 1 = `.text`). `puts` is `NOTYPE`/`GLOBAL`/`Ndx UND` — referenced but not defined — exactly the gap PLT/GOT exist to close.

## "What is `call puts@PLT`?"

Took a couple of passes to land correctly. The real disassembly of the *linked* `hello` binary:
```
main:
    lea    0xeac(%rip),%rax
    mov    %rax,%rdi
    call   1050 <puts@plt>
...
0000000000001050 <puts@plt>:
    endbr64
    bnd jmp *0x2f75(%rip)     # 3fd0 <puts@GLIBC_2.2.5>
```
- `puts`'s real code lives in `libc.so`, loaded at an address that isn't fixed and can change between runs (ASLR) — nothing in `hello.o` could ever hardcode it.
- The linker instead builds a small forwarding stub *inside the executable* — the **PLT** (Procedure Linkage Table), one entry per external function actually called.
- That stub does one `jmp` through a slot in the **GOT** (Global Offset Table) — an address filled in once the loader finds `libc.so` in memory.

## "Is the PLT in my executable, not my object file?"

Checked directly — `hello.o` has zero PLT code:
```
$ objdump -d hello.o | grep -i -A3 "call.*puts"
(nothing)

$ readelf -r hello.o
Relocation section '.rela.text' contains 2 entries:
  ...
  000000000013  ...  R_X86_64_PLT32  puts - 4
```
`hello.o` has only a *relocation note*: "patch this call once you build the final executable and know where the PLT entry for `puts` will be." The PLT itself only gets constructed by the linker while producing `hello`. Confirmed — it's a property of the linked executable, not the object file.

## "Is the GOT in the data section? Filled by the OS?"

Both close, both needed a correction:

- **Not literally `.data`** — GOT is its own section, `.got`, at its own address:
  ```
  [24] .got   PROGBITS  0x3fb8
  [25] .data  PROGBITS  0x4000
  [26] .bss   NOBITS    0x4010
  ```
  It's writable *data* in spirit, just not merged into `.data` itself.
- **Not literally "the OS"** — the thing writing it is a specific userspace program, the dynamic linker/loader (`ld.so`), which the kernel launches to set up the process before `main` runs. And on this exact binary:
  ```
  $ readelf -d hello | grep -i bind
  0x1e (FLAGS)  BIND_NOW
  ```
  `BIND_NOW` means this system resolves the GOT fully at startup, not lazily on the first call — the "resolve lazily on first use" behavior often described as the default elsewhere isn't actually what's happening here.

## "Where did `1050` come from?"

Not a special number — arithmetic from sequential section layout:
```
[12] .init     0x1000  size 0x1b
[13] .plt      0x1020  size 0x20
[14] .plt.got  0x1040  size 0x10
[15] .plt.sec  0x1050  size 0x10   <- puts@plt lives here
[16] .text     0x1060
```
`0x1040 + 0x10 = 0x1050` — `.plt.sec` starts exactly where `.plt.got` ends. A different gcc version or a different set of linked libraries would land it at a different number entirely; the number itself carries no meaning.

## Takeaways

- "Object" in "object file" is mostly historical naming, but ELF also uses it literally — `OBJECT` is a real symbol type, alongside `FUNC`.
- A `.o` file's symbol table records what it defines (`Ndx` = a real section) vs. merely references (`Ndx UND`) — that gap is exactly what linking exists to close.
- PLT and GOT both exist *because* a shared library's real address isn't known until runtime: PLT is link-time-built code inside your executable, GOT is runtime-filled data in its own section.
- None of the specific addresses (`1050`, `1040`, ...) are meaningful on their own — they're just wherever sequential section layout happened to land them for this particular build.

# 00 — Q&A: headers, `#include`, and why `greet.c` includes its own header

From the `main.c`/`greet.c`/`greet.h` lab (`labs/`).

## "Why can't `main.c` just `#include "greet.c"` instead of using a header?"

It technically *can* — `#include` is a dumb textual paste, the preprocessor doesn't care what extension the file has. But it breaks the two things separate compilation is for:

- **Symbol collision.** If `main.c` pastes in all of `greet.c`, `main.o` now contains a full copy of `greet()`'s compiled code. Separately compiling `greet.c` into `greet.o` produces *another* copy of the same symbol. Link the two together and the linker sees `greet` defined twice and refuses.
- **Loses the point of separate compilation.** The whole reason `.c` files compile independently (`notes.md`, "Why separate compilation matters") is that changing one file's *implementation* shouldn't force everything that calls it to recompile. If `main.c` textually includes `greet.c`, then `main.c`'s preprocessed output changes every time `greet.c`'s body changes, so `make` would recompile `main.o` too — even though nothing about *how main calls greet* changed.

The header's job is to give callers only the **contract** — name, return type, parameter types (`void greet(const char *name);`) — enough for the compiler to check a call is well-formed and generate correct calling code, without handing over the implementation. The linker resolves the real address later, once it sees `greet.o`.

## "main.c includes greet.h — fine, it calls greet(). But why does greet.c *also* include greet.h? It already has the body right there."

This was the confusing one. `greet.c` doesn't need the declaration to know how to call `greet` — it isn't calling it, it's defining it. The actual reason: including the header lets the compiler **cross-check the definition against the declaration**, in the same translation unit, immediately.

Concrete failure mode without that check: someone edits `greet.h` to add a parameter —
```c
void greet(const char *name, int shout);
```
— but forgets to update `greet.c`'s definition, which still reads
```c
void greet(const char *name) { ... }
```
If `greet.c` didn't include `greet.h`, `gcc -c greet.c` compiles this old signature just fine in isolation — nothing there contradicts it. Meanwhile `main.c` (which *does* include the updated header) compiles calls assuming two arguments. The two files now silently disagree about `greet`'s contract, and it either fails weirdly at link time or, worse, misbehaves at runtime.

With `greet.c` including its own header, both the declaration and the definition are visible to the compiler at once, so a mismatch is a compile error right at the source of the mistake, not a mystery later. Rule of thumb: **the `.c` file that defines a function always includes its own header** — not because it needs to, but so the header acts as a single checked source of truth for every file involved, definer included.

## Takeaways

- A header is a contract (declaration), not an implementation — pasting a `.c` file's body into another `.c` file instead breaks both symbol uniqueness and incremental rebuilds.
- The definer including its own header isn't redundant — it's what makes a declaration/definition mismatch a compile-time error instead of a silent cross-file disagreement.

# 00 — Q&A: what's actually in an executable, and proving ASLR without gdb

## "Does the linker resolve `greet()` the same way it resolved `puts@PLT`? Is `greet` UND in the symbol table too?"

Checked `main.o` directly instead of guessing:
```
$ nm main.o
                 U greet
0000000000000000 T main

$ readelf -r main.o
R_X86_64_PLT32    0000000000000000 greet - 4
```
Yes — `greet` is `U` (undefined) in `main.o`, same as `puts` was, and it even gets the identical `R_X86_64_PLT32` relocation type. But the *linked* binary tells a different story: disassembling `greet_app` shows `call 118a <greet>` going straight to `greet`'s real code, no `greet@plt` stub anywhere. Reason: `greet` ends up statically linked into the same executable as `main.o` (via `greet.o`), so its address is fully known once the linker finishes — no shared-library runtime uncertainty to route around. `puts` lives in `libc.so`, mapped at an address only known at load time, so *that* one needs the PLT/GOT indirection. Same relocation type in the `.o` file doesn't guarantee the same resolution mechanism — it just means "resolve at link time if you can, fall back to PLT if you can't."

## "What kind of addresses are these — is the executable just bytes the CPU runs through?"

No — the file is a structured container: instruction bytes (`.text`), the data those instructions reference (`.rodata`/`.data`/`.bss`), and — the part that answers this question — a table of **program headers** telling the *loader* where to map each piece:
```
$ readelf -l greet_app
Type: DYN (Position-Independent Executable file)
  LOAD  VirtAddr 0x1000  ...  R E   <- .text
  LOAD  VirtAddr 0x0000  ...  R     <- .rodata
  LOAD  VirtAddr 0x3db8  ...  RW    <- data/GOT
```
Every address in the disassembly (`118a`, `1149`, ...) is one of these virtual addresses — private to the process, translated to physical RAM by the MMU on every access, never a raw file offset or a physical address. Full writeup landed in `notes.md` ("From linked file to running process").

## Trying to prove ASLR, and hitting a missing tool

Wanted to show the loaded address actually changes between runs of the same binary. First attempt:
```
$ gdb -q -batch -ex "break main" -ex run ./greet_app
bash: gdb: command not found
```
Not installed in this environment. `greet_app` itself also runs and exits too fast (~microseconds) to catch mid-flight with a plain `& sleep 0.2; cat /proc/$pid/maps` race — it was already gone by the time the sleep finished.

Fix: wrote a disposable binary that just sleeps, so there's a large enough window to actually read its own mapping while it's alive:
```c
// aslr_check.c
#include <unistd.h>
int main(void) { sleep(2); return 0; }
```
Ran it three separate times, grabbing its base address from `/proc/<pid>/maps` each time:
```
run 1: 55aab68a3000
run 2: 6236bb883000
run 3: 5c47323b8000
```
Three different bases, same compiled file — confirms ASLR directly rather than citing it. Full explanation (why PIE/`Type: DYN` enables this, how it composes with GOT) is in `../virtual-memory/notes.md` under "ASLR."

## Takeaways

- Same relocation type (`R_X86_64_PLT32`) in a `.o` file doesn't imply the same runtime mechanism — whether it resolves to a direct call or a PLT/GOT stub depends on whether the target's address is knowable at link time.
- An executable's addresses are virtual, not physical or raw file offsets — the program headers tell the loader where to map each segment, and that mapping is what the process address-space diagram elsewhere in these notes is actually describing.
- Missing tooling (`gdb`) isn't a dead end — a 3-line disposable binary plus `/proc/<pid>/maps` was enough to directly prove ASLR instead of taking the term on faith.
