# Lab 01.1 — Process info & memory layout

## Goal

Get comfortable reading process identity and inspecting where different kinds of variables actually live in memory.

## Task

Complete `pid_info.c`:

1. Print the process's PID and PPID (`getpid()`, `getppid()`).
2. There's a global variable with an initializer and one without already declared — print both of their addresses.
3. Allocate memory with `malloc()` (heap) and declare a local variable (stack) inside `main`. Print both addresses.
4. Print the address of `main` itself (text segment) — cast the function pointer to `(void *)`.
5. Order the five addresses from lowest to highest and check it against the layout diagram in `notes/processes/notes.md`.

## Verify

- `make run`
- Compare the printed order of addresses against the diagram in the notes — text should be lowest, stack should be highest (heap and the data/bss globals should sit in between).
- Bonus: add a `getchar()` right before `return 0;` so the program pauses, then in another terminal run `cat /proc/<pid>/maps` (find the PID with `pgrep pid_info`) and match the ranges up against what you printed.
