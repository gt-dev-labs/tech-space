# Lab 03 — Watching pages get faulted in

## Part 1 — Demand paging (`demand_paging.c`)

`malloc()` reserving virtual address space doesn't mean physical memory is actually behind it yet. Complete `demand_paging.c`:

1. Implement `print_rss(label)`: open `/proc/self/status`, find the line starting with `VmRSS:` (this is your process's **resident set size** — physical memory actually mapped in right now, in KB), and print it prefixed with `label`.
2. Call it once right after `malloc()`ing a large buffer, before touching any of it.
3. Then write one byte to every page in the buffer — writing anywhere in a page faults the *whole page* in, so one byte per 4096 is enough; you don't need to fill the whole buffer.
4. Call `print_rss()` again after that.

## Verify (Part 1)

`make run-demand`. You should see RSS barely change right after `malloc()` (the pages aren't resident yet — this is demand paging), then jump up close to the buffer's full size after you've touched every page.

## Part 2 — Copy-on-write faults, counted (`cow_faults.c`)

Complete `cow_faults.c`. The parent allocates and fully writes a large buffer (so every page is already resident and *not* shared with anything else yet), then `fork()`s:

1. In the **child**: record `getrusage(RUSAGE_SELF, ...)`'s `ru_minflt` (minor fault count) right after `fork()`, before touching the buffer again. Then write to every page in the buffer again (e.g. `memset` it to a different byte). Record `ru_minflt` once more and print the difference.
2. In the **parent**: `waitpid()` for the child, then check the parent's *own* `ru_minflt` before vs. after the child ran — it should barely move, since the parent never wrote to anything after the fork.

## Verify (Part 2)

`make run-cow`. The child's minor-fault jump should land in the neighborhood of `buffer size / 4096` (e.g. ~25,600 for a 100 MB buffer) — that's the actual count of copy-on-write page faults, one per page the child wrote to. The parent's fault count should stay essentially flat, since none of its pages ever got copied — it never wrote to any of them after forking.
