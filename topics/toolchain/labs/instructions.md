# Lab 00 — Compiling and running C by hand

No `Makefile` is provided for this lab — writing one yourself is Part 3.

## Part 1 — The four stages, manually (`hello.c`)

Run each step yourself and look at the intermediate output:

1. `gcc -E hello.c -o hello.i` — preprocessing only. Open `hello.i` — `#include <stdio.h>` has been replaced by the actual contents of the header (hundreds of lines).
2. `gcc -S hello.i -o hello.s` — compile the preprocessed source to assembly. Open `hello.s` — you don't need to understand the assembly, just confirm it's text, not machine code.
3. `gcc -c hello.s -o hello.o` — assemble to an object file. Try `cat hello.o` (binary garbage) and `file hello.o` (confirms what it is).
4. `gcc hello.o -o hello` — link to a final executable. Run `./hello`.
5. Now do it the normal way: `gcc hello.c -o hello`. Same four steps, one command.

## Part 2 — Multiple files, still by hand (`main.c`, `greet.c`, `greet.h`)

This project has two source files and one header. `greet.h` **declares** `greet()`; `greet.c` **defines** it; `main.c` calls it — the same declare/define split every header in a real C codebase (nginx included) uses.

1. Compile each `.c` file to an object file, without linking:
   - `gcc -Wall -Wextra -c main.c -o main.o`
   - `gcc -Wall -Wextra -c greet.c -o greet.o`
2. Link the two object files into one executable:
   - `gcc main.o greet.o -o greet_app`
3. Run it: `./greet_app` and `./greet_app YourName`.
4. Delete just `greet.o` and re-run only the link command from step 2 (don't recompile `greet.c`) — it fails, missing symbols. Now re-run only `gcc -Wall -Wextra -c greet.c -o greet.o`, then the link again — works. That's the payoff of separate compilation: only the file that changed needs recompiling, then re-link.

## Part 3 — Write your own Makefile

Write a `Makefile` for the `main.c` / `greet.c` / `greet.h` project that supports:

- `make` (or `make greet_app`) — builds `greet_app`, recompiling only what changed
- `make run` — builds (if needed) and runs `greet_app`
- `make clean` — removes `greet_app` and all `.o` files

Get these right:
- `greet_app` should depend on `main.o` and `greet.o`
- `main.o` and `greet.o` should each also depend on `greet.h` — headers are dependencies of every `.c` file that includes them
- use `CC` / `CFLAGS` variables, like the Makefiles elsewhere in this repo
- `run` and `clean` should be declared `.PHONY`

## Verify

- Parts 1 and 2 run correctly using only manual `gcc` invocations — no `make` involved.
- Check your Part 3 Makefile actually tracks dependencies correctly: `touch greet.c && make` should only recompile `greet.c` and re-link, not touch `main.o`. `touch greet.h && make` should recompile *both* `.o` files.
- Once this lab is done, go back and reread `../processes/labs/*/Makefile` — they should look like plain, boring, readable files now instead of magic.
