# File Descriptors
Tags: #os #systems-programming
Prerequisites: [Processes](../processes/notes.md), [System calls](../system-calls/notes.md)

A **file descriptor (FD)** is a small, non-negative integer that a process uses as a handle for an open kernel-managed object.

The integer itself is not the file, socket, pipe, terminal, or kernel pointer. It is best understood as an index into a **per-process file-descriptor table** maintained by the kernel.

```text
process
  |
  | fd = 3
  v
file-descriptor table
  [0]
  [1]
  [2]
  [3] ----------> open-file object in the kernel
                        |
                        |-- current offset
                        |-- access/status flags
                        `-- reference to underlying object
                                   |
                                   v
                              inode / socket /
                              pipe / device / ...
```

For example:

```c
int fd = open("data.txt", O_RDONLY);
```

If `open()` returns `3`, then `3` means roughly:

> Entry 3 in this process's file-descriptor table refers to the kernel object for this open instance.

A later syscall such as:

```c
read(3, buf, 100);
```

causes the kernel to identify the current process, look up entry 3 in that process's FD table, obtain the corresponding open kernel object, and perform the read through it.

## The main layers

### 1. The descriptor number

`3` is only a process-local integer. Another process can also have an FD `3` referring to something completely unrelated.

### 2. The per-process file-descriptor table

Conceptually, the table behaves like:

```text
fd_table[3] -> open-file object
```

The exact Linux implementation is more involved, but this is the useful low-level mental model.

### 3. The open-file object

For a regular file, Linux has a kernel object roughly corresponding to `struct file`. It represents an **open instance** and stores state associated with that opening, such as the current file offset and status flags.

Example:

```text
offset = 0
flags  = O_RDONLY
object = data.txt
```

After reading 10 bytes through that open-file object, its offset is typically 10.

### 4. The underlying object

The open-file object eventually refers to the actual kind of resource being accessed. For a regular file the chain is approximately:

```text
fd -> open-file object -> inode -> filesystem data
```

But an FD can also refer to many other kernel-managed objects:

```text
fd -> regular file
fd -> socket
fd -> pipe
fd -> terminal
fd -> device
fd -> eventfd
fd -> epoll instance
```

This unified integer-handle interface is a major Unix design idea: many very different resources can be manipulated through APIs such as `read(fd, ...)`, `write(fd, ...)`, and `close(fd)`.

## File descriptor vs. open-file description

These are not the same thing.

Two descriptor numbers can refer to the same open-file object:

```text
fd 3 --\
        +--> same open-file object --> underlying file
fd 7 --/
```

This can happen after operations such as `dup()`, and related sharing also matters with `fork()`. Because the descriptors share the same open-file object, they can also share state such as the current file offset.

The core mental model is therefore:

> A file descriptor is a process-local integer handle that the kernel uses to find an open kernel object.
