/*
CASE ??? : test
CASE 0: pipe
CASE 1 : ful pipe
CASE 2 : SIGPIPE
CASE 3 : use pipe for synchronization
CASE 4 ; popen (read), pclose
CASE 5 : popen (write), pclose
CASE 6 : FIFO writer
CASE 7 : FIFO reader
CASE 8 : Message Queue system V
CASE 9 : Message Queue Posix
*/

#define CASE 9

#if CASE == 0
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

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
        /* child process close the write end
           child process simply read
        */
        // close(fd1[1]);
        printf("Before read\n");
        char buffer[100];

        size_t n = read(fd1[0], buffer, sizeof(buffer) - 1);

        if (n == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
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
#elif CASE == 1
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    int n = 0;
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    const char *msg = "HELLO HELLO HELLO HELLO HELLO HELLO";
    while (1)
    {
        write(fd[1], msg, strlen(msg));
        printf("write %d time\n", n);
        n++;
    }
    return 0;
}
#elif CASE == 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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
#elif CASE == 3
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int p_fd[2];

    printf("parent start\n");

    if (pipe(p_fd) != 0)
    {
        perror("pipe");
        return -1;
    }

    switch (fork())
    {
    case -1:
        /* error */
        perror("fork");
        return -2;
        break;

    case 0:
        /* child close the read end */
        if (close(p_fd[0]) == -1)
        {
            perror("child close");
            exit(-3);
        }
        /* do something */
        sleep(5);

        printf("child closed the pipe\n");

        /* close the write end */
        if (close(p_fd[1]) == -1)
        {
            perror("child close");
            exit(-3);
        }

        exit(12);
        break;

    default:
        break;
    }

    /* parent close the write end */
    if (close(p_fd[1]) == -1)
    {
        perror("parent close");
        return -4;
    }

    char dummy[100];

    read(p_fd[0], &dummy, 100);

    printf("parent ready to run\n");

    return 0;
}
#elif CASE == 4
#include <stdio.h>

/* using popen to run command passed via argument *argv[] */
int main(int argc, char *argv[])
{
    FILE *file;
    /* run command */
    file = popen("ls", "r");

    if (file == NULL)
    {
        perror("popen");
        return -1;
    }

    char buffer[1024];

    /* print out the result */
    while ((fgets(buffer, sizeof(buffer), file)))
    {
        printf("%s", buffer);
    }

    /* close */
    pclose(file);
    return 0;
}
#elif CASE == 5
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *file;
    /* run command */
    file = popen("grep Hello", "w");

    if (file == NULL)
    {
        perror("popen");
        return -1;
    }

    fprintf(file, "Hello world!\n");
    fprintf(file, "Hello world again!\n");

    /* close */
    pclose(file);
    return 0;
}
#elif CASE == 6
/*
This example will reproduce communication between 2 processes:
The writer:
    write data to fifo named fifo_A_to_B
    read data from fifo named fifo_B_to_A
*/
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    /* create fifo if it does not exist */
    mkfifo("/tmp/fifo_A_to_B", 0666);

    char buffer[512];                      /* buffer save data received */
    const char *msg = "Hello from writer"; /* data to send */

    /* open fifo */
    int fd = open("/tmp/fifo_A_to_B", O_WRONLY);
    int fd1 = open("/tmp/fifo_B_to_A", O_RDONLY);

    /* the loop comunication */
    while (1)
    {
        write(fd, msg, strlen(msg));

        int n = read(fd1, buffer, sizeof(buffer) - 1);

        buffer[n] = '\0';

        printf("%s\n", buffer);
        sleep(1);
    }

    close(fd);
    close(fd1);
    return 0;
}
#elif CASE == 7
/*
This example will reproduce communication between 2 processes:
The reader:
    read data from fifo named fifo_A_to_B
    write data to fifo named fifo_B_to_A
*/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    /* create fifo if it does not exist */
    mkfifo("/tmp/fifo_B_to_A", 0666);

    char buffer[512];                      /* buffer save data received */
    const char *msg = "Hello from reader"; /* data to send */

    /* open fifo */
    int fd = open("/tmp/fifo_A_to_B", O_RDONLY);
    int fd1 = open("/tmp/fifo_B_to_A", O_WRONLY);

    /* the loop comunication */
    while (1)
    {
        int n = read(fd, buffer, sizeof(buffer) - 1);

        buffer[n] = '\0';

        printf("%s\n", buffer);

        write(fd1, msg, strlen(msg));
        sleep(1);
    }

    close(fd);
    close(fd1);
    return 0;
}
#elif CASE == 8
/*
/* Mode bits for `msgget', `semget', and `shmget'.  /
#define IPC_CREAT	01000		/* Create key if key does not exist. /
#define IPC_EXCL	02000		/* Fail if key exists.  /
#define IPC_NOWAIT	04000		/* Return error on wait.  /

/* Control commands for `msgctl', `semctl', and `shmctl'.  /
#define IPC_RMID	0		/* Remove identifier.  /
#define IPC_SET		1		/* Set `ipc_perm' options.  /
#define IPC_STAT	2		/* Get `ipc_perm' options.  /
*/

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "type.h"

int main(int argc, char *argv[])
{
    key_t key = 1234;

    int msg_id = msgget(key, IPC_CREAT | 0660);

    if (msg_id == -1)
    {
        perror("mssget");
        return -1;
    }

    /* create message to send */
    struct message msg1;
    // msg1.msg_type = 1;
    strcpy(msg1.msg_text, "HiHiHi HeHeHe");

    /* send message */
    if (msgsnd(msg_id, &msg1, strlen(msg1.msg_text) + 1, 0) == -1)
    {
        perror("msgsnd");
        return -3;
    }
    /***********************************************************/

    /* receive message */
    struct message receive;
    if (msgrcv(msg_id, &receive, sizeof(receive.msg_text), 0, 0) == -1)
    {
        perror("msgrcv");
        return -2;
    }

    struct msqid_ds ds;
    msgctl(msg_id, IPC_STAT, &ds);

    /* print out information */
    print_info(key, msg_id, &ds, receive.msg_text, NULL);

    // getchar();
    return 0;
}
#elif CASE == 9
#include <mqueue.h>

int main(int argc, char *argv[])
{

    struct mq_attr attr;

    attr.mq_flags = O_CREAT;
    attr.mq_maxmsg = 10;    /* maximum number of message */
    attr.mq_msgsize = 50;   /* maximum message size */
    
    mqd_t mq = mq_open("/hihi", O_CREAT);
    
    mq_getattr(mq, &attr);

    return 0;
}
#else
#include <stdio.h>
#include <string.h>

int main()
{
    FILE *file = fopen("document.txt", "wb");

    if (file == NULL)
    {
        return -1;
    }
    const char *msg = "hehehehehe";

    fwrite(msg, sizeof(char), strlen(msg), file);

    fclose(file);
    return 0;
}
#endif