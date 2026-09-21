/*
CASE 0 : Message System V
CASE 1 : Message POSIX
CASE 2 : Shared memory System V
CASE 3 : FIFOs
*/

#define CASE 2

#if CASE == 0
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

    /* receive message */
    char buffer[100];
    struct message receive;
    if (msgrcv(msg_id, buffer, sizeof(buffer), 0, 0) == -1)
    {
        perror("msgrcv");
        return -2;
    }

    /* identifier */
    struct msqid_ds ds;
    msgctl(msg_id, IPC_STAT, &ds);

    /* print out information */
    print_info(key, msg_id, &ds, buffer, NULL);
    // getchar();
    return 0;
}
#elif CASE == 1
#include <stdio.h>
#include <mqueue.h>

int status; /* stored the return value of function to check error */

int main(int argc, char *argv[])
{
    struct mq_attr *get_attr;

    mqd_t mq = mq_open("/hihi", O_RDWR);

    if (mq == -1)
    {
        perror("mq_open");
        return -1;
    }

    /* print out the message attributes */
    // status = mq_getattr(mq, get_attr);

    // if(status == -1)
    // {
    //     perror("mq_getattr");
    //     return -4;
    // }

    // printf("mq_flags: %ld\n", get_attr->mq_flags);
    // printf("mq_maxmsg: %ld\n", get_attr->mq_maxmsg);
    // printf("mq_msgsize: %ld\n", get_attr->mq_msgsize);
    // printf("mq_curmsgs: %ld\n", get_attr->mq_curmsgs);

    /* receive message */
    char buffer[300];
    int prio;

    status = mq_receive(mq, buffer, sizeof(buffer), &prio);

    if (status == -1)
    {
        perror("mq_receive");
        return -3;
    }

    printf("received: %s\nwith priority: %d\n", buffer, prio);
    /**************************************************/

    return 0;
}
#elif CASE == 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "type.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int ret; /* stored the return value of function to check error */
int shm_id;
int sem_id;
struct data *ptr;
struct sembuf sem_opr[2];

void handle(int sig)
{
    /* detach */
    if (shmdt(ptr) == -1)
    {
        perror("shmdt");
    }

    exit(123);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle);

    /* generate the key */
    // key_t key = ftok("/tmp/myshare", 'A');

    /* create the shared memory segment */
    shm_id = shmget(SHARED_MEMORY_KEY, SEGMENT_SIZE, 0666);

    if (shm_id == -1) /* check error */
    {
        perror("shmget");
        return -1;
    }
    printf("Shared memory id: %d\n", shm_id);

    /* attach */
    ptr = shmat(shm_id, NULL, 0);
    if ((void *)ptr == (void *)-1) /* check error */
    {
        perror("shmat");
        return -1;
    }
    printf("Shared memory address: %p\n", ptr);

    /* get semaphore */
    sem_id = semget(SEMAPHORE_KEY, 3, 0666);

    if (sem_id == -1) /* check error */
    {
        perror("semget");
        return -1;
    }
    printf("Semaphore id: %d\n", sem_id);

    /* create semaphore operation */
    sem_opr[0].sem_flg = 0;
    sem_opr[0].sem_num = 0;
    sem_opr[0].sem_op = -1;

    sem_opr[1].sem_flg = 0;
    sem_opr[1].sem_num = 1;
    sem_opr[1].sem_op = 1;

    /* loop */
    while (1)
    {
        semop(sem_id, &sem_opr[0], 1);
        printf("client 1 received: %s\n", ptr->text);
        ptr->money -= 50;
        sprintf(ptr->text, "client consume 50 money. Now left: %d", ptr->money);
        // printf("client consume 50 money. Now left: %d\n", ptr->money);
        semop(sem_id, &sem_opr[1], 1);
        sleep(1);
    }

    return 0;
}
#elif CASE == 3
/*
This example will reproduce communication between 2 processes:
The reader:
    read data from fifo named fifo_A_to_B
    write data to fifo named fifo_B_to_A
*/
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

int fd_read;
int fd_send;

/* function handle signal Ctrl + C */
void handle(int sig)
{
    /* close fd */
    close(fd_read);
    close(fd_send);

    exit(123);
}

/*
function read the message entered via the keyboard then send to client
*/
void *read_input(void *arg)
{
    char msg[512]; /* data to send */

    /* the loop comunication */
    while (1)
    {
        /* get input message from keyboard */
        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            printf("ERROR: get input from keyboard.\n");
            return NULL;
        }
        msg[strcspn(msg, "\n")] = '\0';

        write(fd_send, msg, strlen(msg));
    }
    return NULL;
}

/*
function receive message sent from client
*/
void *rev_message(void *arg)
{
    char buffer[512]; /* buffer save data received */

    while (1)
    {
        int n = read(fd_read, buffer, sizeof(buffer) - 1);

        buffer[n] = '\0';

        printf("%s\n", buffer);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle);

    /* open fifo */
    fd_read = open("/tmp/fifo_A_to_B", O_RDONLY);
    fd_send = open("/tmp/fifo_B_to_A", O_WRONLY);

    pthread_t thread1, thread2;

    /* create thread */
    if (pthread_create(&thread1, NULL, read_input, NULL) != 0)
    {
        printf("Error create thread 1.\n");
        return -1;
    }

    if (pthread_create(&thread2, NULL, rev_message, NULL) != 0)
    {
        printf("Error create thread 2.\n");
        return -1;
    }

    printf("client started.\n");

    /* join thread */
    if (pthread_join(thread1, NULL))
    {
        printf("Error join thread 1.\n");
        return -1;
    }

    if (pthread_join(thread2, NULL))
    {
        printf("Error join thread 2.\n");
        return -1;
    }

    return 0;
}
#endif