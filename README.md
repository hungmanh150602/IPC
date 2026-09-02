# Overview

```text
                         PROCESSES
               ┌──────────────┴──────────────┐
               │                             │
          Process A                     Process B
               │                             │
               └────────── IPC ──────────────┘
                              │
               ┌────────────┼────────────┐
               │            │            │
               Pipe       Message       Shared
                         Queue         Memory
               │            │            │
               └────────────┼────────────┘
                              │
                         Synchronization
                              │
                    ┌──────┴──────┐
                    │             │
               Semaphore       Signal
```

You can view IPC as an evolutionary process:

```text
               fork()
               │
               │ related processes
               ▼
               PIPE
               │
               │ need unrelated processes
               ▼
               FIFO
               │
               │ need structured messages
               ▼
               MESSAGE QUEUE
               │
               │ need very fast shared data
               ▼
               SHARED MEMORY
               │
               │ but now synchronization needed
               ▼
               SEMAPHORE
```

# 1. PIPE

A pipe is an IPC (Inter-Process Communication) mechanism that allows one process to transmit a byte stream to another process via the kernel.

```text
          Process A                         Process B
          │                                  │
          │ write()                          │ read()
          │                                  │
          ▼                                  ▲
          ┌──────────────────────────────────────────┐
          │                  KERNEL                  │
          │                                          │
          │             PIPE BUFFER                  │
          │                                          │
          └──────────────────────────────────────────┘
```

## Why do PIPE exist?

Each process has its own virtual address space:

```text
Process A                  Process B

┌─────────────┐            ┌─────────────┐
│ Stack       │            │ Stack       │
│ Heap        │            │ Heap        │
│ Data        │            │ Data        │
│ Code        │            │ Code        │
└─────────────┘            └─────────────┘
```

A cannot simply go ahead and do:

```c
B->variable = 100;
```

because the variable resides in B's address space.

It is primarily suitable for related processes, particularly those linked via `fork()`.

Prototype:

```c
#include <unistd.h>

int pipe(int pipefd[2]);
```

After Pipe success:

```text
fd[0] = read end
fd[1] = write end
```

***The output of fd[1] is the input for fd[0].***

![alt text](image.png)
