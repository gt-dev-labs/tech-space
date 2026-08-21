# Lab 01.4 — Zombies and orphans

## Goal

Deliberately create a zombie process and an orphan process, and observe them with `ps`.

## Part A — Zombie (`zombie.c`)

1. `fork()`.
2. Child: print its own PID, then `_exit(0)` immediately.
3. Parent: print a message telling you to go check `ps`, then `sleep(30)` — **without** calling `wait()`.
4. While the parent sleeps, in another terminal run `ps -o pid,ppid,stat,cmd -C zombie` and confirm the child shows `STAT` = `Z` (zombie / `<defunct>`).

## Part B — Orphan (`orphan.c`)

1. `fork()`.
2. Parent: print its own PID, then exit almost immediately — no `wait()`, no sleep.
3. Child: print its own PID and PPID, `sleep(2)`, then print its PPID again.
4. Confirm the child's PPID changes from the original parent's PID to `1` (or a reaper process) once the parent has exited.

## Verify

- Run each program, using `ps --forest` or `ps -o pid,ppid,stat` in another terminal while it's running.
- In a comment at the top of `zombie.c`, write one sentence on why zombies are harmless in small numbers but a real problem for a long-running process (like a server) that spawns children without ever reaping them.
