# Lab 01.3 — fork + exec + wait

## Goal

Implement the fork/exec/wait pattern every shell uses to run external commands.

## Task

Complete `fork_exec.c` so it runs the command given in `argv[1..]` (e.g. `./fork_exec ls -la`):

1. `fork()`.
2. In the child: call `execvp(argv[1], &argv[1])`. If it returns at all, the exec failed — print an error with `perror("execvp")` and `_exit(127)`.
3. In the parent: `waitpid()` for the child, then inspect the status:
   - if it exited normally, print the exit code (`WIFEXITED` / `WEXITSTATUS`)
   - if it was killed by a signal, print which one (`WIFSIGNALED` / `WTERMSIG`)
4. Handle `fork() == -1`.

## Verify

- `make`
- `./fork_exec ls -la` → runs `ls -la`, then prints its exit code (0)
- `./fork_exec false` → should report exit code 1
- `./fork_exec bash -c 'kill -9 $$'` → should report "killed by signal 9"
- `./fork_exec /no/such/binary` → should report the exec failure via `perror`, not crash
