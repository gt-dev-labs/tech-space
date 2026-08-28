# Node.js

Node.js internals — the event loop, libuv, and how async I/O actually works under the hood — with nginx's own event-loop architecture included as a comparison example, not as an equal co-subject. A curated plan through the shared topic pool in `../topics/`.

## Plan

1. Node.js/libuv event loop internals, with nginx's master/worker + epoll model as a contrasting example — *not built yet*

**Builds on:** the [operating-systems](operating-systems.md) plan's processes, threads, and I/O multiplexing steps — this is largely a case study applying that material to two real, well-known systems, not a from-scratch reintroduction of the underlying OS mechanisms.

Originally planned as a topic inside the operating-systems plan ("case studies: nginx & Node.js/libuv"), split out into its own plan since Node.js internals is substantial enough to warrant its own roadmap, with nginx folded in as an example rather than a second full subject.
