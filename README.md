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

> ***A pipe is an IPC (Inter-Process Communication) mechanism that allows one process to transmit a byte stream to another process via the kernel.***

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

## 1.1 Why do PIPE exist?

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

Process A cannot simply go ahead and do:

```c
B->variable = 100;
```

because the variable resides in B's address space. It requires an intermediary mechanism. And that is **Pipe**.

**What is the pipe suitable for?**

Pipe is suited for:

- shell pipelines
- parent-child communication
- simple producer-consumer scenarios
- passing stdin/stdout
- data streaming

Not ideal if you need:

- complex message structures
- random access
- large shared datasets
- multiple clients requiring flexible communication

Pipe is primarily suitable for related processes, particularly those linked via `fork()`.

Prototype:

```c
#include <unistd.h>

int pipe(int fd[2]);
```

Return:

- 0: if successfull
- -1: if not

After Pipe success:  
Two file descriptors are stored in fd;  
bytes written on fd[1] can be read from fd[0].

```text
fd[0] = read end
fd[1] = write end
```

***The output of fd[1] is the input for fd[0].***

![alt text](image.png)

Example Pipe and Fork:

Classic patern:

```c
int fd[2];

pipe(fd);

pid_t pid = fork();
```

```c
int main(void)
{
    int fd[2];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) /* child process */
    {
        /* child process close the write end
           child process simply read
        */
        close(fd[1]);
        char buffer[100];

        size_t n = read(fd[0], buffer, sizeof(buffer) - 1);

        if (n == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        buffer[n] = '\0';
        printf("%lu Child received: %s\n", n, buffer);
        close(fd[0]);
    }
    else /* parent process */
    {
        /* parent process close the read end
           parent process simply write
        */
        close(fd[0]); 
        const char *msg = "Hello from Parent";

        write(fd[1], msg, strlen(msg));
        close(fd[1]);

        wait(NULL);
    }
    return 0;
}
```

```text
17 Child received: Hello from Parent
```

We notice the appearance of the number 17; that is the return value of the `read` function, and we will discuss it later.

If we want to transfer data between parent and child process, we have to use two pipe.

![alt text](image-1.png)

## 1.2 Pipe has limited capacity

If pipe is full, `write()` can be blocked.

```c
const char *msg = "HELLO HELLO HELLO HELLO HELLO HELLO";
while (1)
{
    write(fd[1], msg, strlen(msg));
    printf("write %d time\n", n);
    n++;
}
```

```text
write 1868 time
write 1869 time
write 1870 time
write 1871 time
^C
```

I can write a `msg` string to pipe 1871 times then I can't. Because the default maximum size of pipe is 64Kb, size of `msg` is 35 byte and I write 1871 times, total of size is 65485 byte. It is 64kb.

## 1.3 Blocking

A different example about the child process does not close the write end, so, instead the parent have done writing but the child still waiting for read. It is **blocking**.

```c
int main(void)
{
    int fd1[2];

    if (pipe(fd1) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) /* child process */
    {
        /* child process does not close the write end
           child process simply read and can be blocked
        */
        // close(fd1[1]);
        printf("Before read\n");
        char buffer[100];

        size_t n;

        while ((n = read(fd1[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            printf("reading...\n");
        }

        printf("After read\n");

        buffer[n] = '\0';
        printf("%lu Child received: %s\n", n, buffer);

        close(fd1[0]);
    }
    else /* parent process */
    {
        /* parent process close the read end
           parent process simply write
        */
        close(fd1[0]);
        sleep(5);
        const char *msg = "Hello from Parent";
        write(fd1[1], msg, strlen(msg));
        close(fd1[1]);
        wait(NULL);
    }
    return 0;
}
```

```text
Before read
reading...
^C
```

## 1.4 SIGPIPE

*SIGPIPE* is a signal sent when we writing data to a pipe that has no read end.

Normally, If the process does not handle this signal, the default behavior for `SIGPIPE` is to terminate the process.

If we ignor or create function to handle it. we can see the return value:

Example:

```c
void signal_handler(int sig)
{
    if (sig == SIGPIPE)
    {
        printf("write fail with exit signal: SIGPIPE\n");
    }
    return;
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, signal_handler);
    int fd[2];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    close(fd[0]);

    const char *msg = "HELLO HELLO HELLO HELLO HELLO HELLO";
    /*
    write return the number of byte written
    return -1 if error
    */
    int ret = write(fd[1], msg, strlen(msg));
    printf("write done with return: %d\n", ret);
    
    return 0;
}
```

```text
write fail with exit signal: SIGPIPE
write done with return; -1
```

## 1.3 popen and pclose
