# Lab 01.5 — Mini shell (capstone)

## Goal

Combine everything from this module into a tiny interactive shell: read a line, fork, exec, wait, repeat.

## Task

Complete `mini_shell.c`:

1. Loop: print a `> ` prompt, read a line with `fgets`.
2. Exit the loop on EOF or if the trimmed input is `exit`.
3. Tokenize the line into `argv[]` by whitespace (`strtok` is fine — no need to handle quoting). NULL-terminate the array.
4. If there are no tokens (blank line), loop again without forking.
5. Bonus built-in: if `argv[0]` is `"cd"`, call `chdir(argv[1])` directly and loop again — a child process changing its own directory wouldn't affect the shell's directory, so `cd` can't be implemented via fork+exec.
6. Otherwise: `fork()`, `execvp(argv[0], argv)` in the child (`_exit(127)` on failure), `waitpid()` in the parent — reuse the pattern from lab 01.3.
7. Print the child's exit status after each command, the way a real shell tracks `$?`.

## Verify

- `make run`
- Try: `ls`, `pwd`, `sleep 2`, `false`, `nonexistent-command`, `exit`
- Bonus: `cd /tmp` followed by `pwd` should print `/tmp`
