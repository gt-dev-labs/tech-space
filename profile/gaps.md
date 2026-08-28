# Knowledge Gaps & Recommendations

A running log of gaps that show up in conversation, with a recommendation for each. Updated as they come up, not request-gated — same living-document treatment as `topics/*/notes.md`. When a gap gets closed (a topic gets built, or you demonstrate you've picked it up), move it to Resolved rather than deleting it — the history of what used to be a gap is useful on its own.

## Open

### Filesystems & storage
No topic exists yet for how files actually get stored — file system implementation, journaling, RAID, SSDs vs. spinning disks. Came up while evaluating OSTEP as a reference: its Persistence section is the largest part of that book, and this repo has nothing covering it at all. Foundational, on par with what's already built for processes/virtual memory.

**Recommendation:** a filesystems topic (or a short sequence of them), added to the `operating-systems` track.

### Security fundamentals (auth, access control, crypto basics)
Also flagged during the OSTEP evaluation — no topic exists. Lower priority than filesystems for your stated goals (nginx/Node/Docker/k8s), but relevant once containers/k8s security comes up for real.

**Recommendation:** revisit once the `docker-kubernetes` track has real content — container/orchestration security is where this would actually get used, rather than as abstract OS theory.

## Resolved

### CPU architecture / registers / calling conventions
Was a real gap at the start of working through assembly output (`hello.s`) — no prior framework for registers, the stack mechanism, or the ABI. Closed by building the `cpu-architecture` topic and its labs (`gdb` register-stepping, the hand-written `raw.s` syscall lab).

### Object files, linking, symbol tables, PLT/GOT
Came up as a long chain of "why is it called that / where did this number come from" questions while reading `hello.o`/`hello`. Closed by the `toolchain` topic and its `qa.md`, which records the actual investigative trail (readelf/objdump evidence, not just conclusions).
