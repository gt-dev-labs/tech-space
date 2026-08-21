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
