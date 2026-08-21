# 01 — Processes

## What is a process?

A process is a running instance of a program: an address space (memory) plus OS-tracked state — registers, open file descriptors, process ID (PID), parent PID (PPID), scheduling info. The kernel keeps one record per process (`task_struct` on Linux); you don't need its fields, just know that everything below is really "what's in that record, and how you can query or change it."

Key identifiers:
- **PID** — unique process ID
- **PPID** — parent's PID (the process that created it)
- get them with `getpid()` / `getppid()`

## Process memory layout (address space)

A process's virtual address space is divided into segments, roughly low to high addresses:

```
high addr  +------------------+
           |      stack       |  <- grows down; local vars, call frames
           |        |         |
           |        v         |
           |------------------|
           |   (free space)   |
           |        ^         |
           |        |         |
           |       heap       |  <- grows up; malloc/free
           |------------------|
           |       bss        |  <- uninitialized globals/statics (zeroed)
           |------------------|
           |   data (init)    |  <- initialized WRITABLE globals/statics
           |------------------|
           |     rodata       |  <- initialized READ-ONLY data (string literals, const globals)
           |------------------|
low addr   |       text       |  <- compiled code (read-only)
           +------------------+
```

- **text** — compiled machine code, read-only, shared between processes running the same binary
- **rodata** — initialized data that's read-only: string literals (e.g. `"hello"`) and `const` globals live here, not in `data`. This is why `char *s = "hello"; s[0] = 'X';` segfaults — that literal is in a read-only segment — while `char s[] = "hello"; s[0] = 'X';` works, since that copies the bytes into a writable array on the stack instead.
- **data** — global/static variables with an explicit initial value, writable
- **bss** — global/static variables with no explicit initializer (zero-filled at load, not stored in the binary)
- **heap** — dynamic memory (`malloc`/`free`), grows toward higher addresses
- **stack** — function call frames and locals, grows toward lower addresses; each thread gets its own stack (module 05)

This is *virtual* memory — every process sees its own private view of this layout; the MMU and page tables map it to physical RAM (or swap). Two processes' `0x400000` are different physical bytes.

Inspect it for real: `cat /proc/<pid>/maps`.

## Process states

Simplified state machine:

- **Running** — actually executing on a CPU
- **Ready/Runnable** — waiting for the scheduler to hand it a CPU
- **Blocked/Sleeping** — waiting on I/O, a lock, a signal, etc.
- **Zombie** — has exited, but the parent hasn't called `wait()` yet, so the kernel keeps its exit status around
- **Terminated** — fully cleaned up once the parent reaps it

## fork() — creating a new process

`fork()` clones the calling process. After it returns, there are *two* processes running the same code from the same point:

- returns `0` in the child
- returns the child's PID in the parent
- returns `-1` on failure (no child created)

The child gets a **copy** of the parent's address space — conceptually. In reality Linux uses **copy-on-write (COW)**: both processes share the same physical pages, marked read-only, until one of them writes to a page — only then does the kernel actually duplicate that page. This makes `fork()` cheap even for large address spaces.

## exec() family — replacing the process image

`fork()` gives you a copy of the *same* program. To run a *different* program in that process, use one of the `exec*()` functions (`execve`, `execvp`, `execl`, ...). `exec()` replaces the calling process's text/data/heap/stack with a new program's — same PID, everything else replaced. On success it never returns (there's no "old" program left to return into); it only returns on failure.

The classic pattern — **fork + exec + wait** — is exactly what a shell does for every command you type:

```c
pid_t pid = fork();
if (pid == 0) {
    execvp(argv[0], argv);   // child becomes the new program
    _exit(127);              // only reached if exec failed
} else {
    waitpid(pid, &status, 0); // parent waits for child
}
```

This is also how nginx's master process starts its worker processes.

## wait() / waitpid() — reaping children

When a child exits, it becomes a zombie until the parent calls `wait()`/`waitpid()`, which retrieves the exit status and lets the kernel free the zombie's remaining bookkeeping. If a parent never waits, zombies accumulate (visible in `ps` as `<defunct>`).

If the *parent* dies before the child, the child is **reparented** to `init`/PID 1 (or a subreaper), which reaps it — that's an **orphan**, not a zombie.

## exit() vs _exit()

- `exit()` (libc) — flushes stdio buffers, runs `atexit()` handlers, then performs the actual termination
- `_exit()` (syscall wrapper) — terminates immediately, no cleanup
- from `main()`, `return n;` is equivalent to `exit(n)`

The exit status is what the parent sees via `wait()`/`waitpid()` (as `WEXITSTATUS(status)`).

## Why this matters later

- nginx's master process `fork()`s its worker processes at startup — no `exec()`, since workers run the same binary, just entering a different code path
- containers don't introduce a new *kind* of process — a "containerized process" is a normal Linux process; module 10 covers what's layered on top of fork/exec (namespaces, cgroups) to make it feel isolated
- zombie/orphan handling matters for anything that supervises subprocesses: nginx's master, the Docker daemon, `init` systems

## Try it yourself

- `ps -ef --forest` — see parent/child relationships
- `cat /proc/self/status` — look at `State`, `PPid`
- `cat /proc/<pid>/maps` while a program runs, to see the memory layout for real
