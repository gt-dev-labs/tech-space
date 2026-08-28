# Lab 01.2 — fork() and copy-on-write

## Goal

See `fork()` in action: two processes running from the same point, with independent copies of memory.

## Task

Complete `fork_basics.c`:

1. A local `int counter = 100;` is declared before `fork()` is called.
2. Call `fork()` and store the result.
3. Handle `pid == -1` (fork failed) — print an error with `perror` and exit.
4. In the child (`pid == 0`): print "child" along with its own PID and PPID, then modify `counter` (e.g. `counter += 1`), then print the new value.
5. In the parent (`pid > 0`): print "parent" along with its own PID and the child's PID (the `pid` you got back), `sleep(1)` so the child prints first, then print `counter` — it should still be 100.

## Verify

- `make run` a few times.
- Confirm the child's printed `counter` is 101 and the parent's is still 100 — that's copy-on-write: the child's write to `counter` triggered a private copy of that memory page, the parent's copy is untouched.
- Note: the `sleep(1)` is only there to make the print order deterministic for this exercise — without it, whether parent or child runs first is up to the scheduler and isn't guaranteed.
