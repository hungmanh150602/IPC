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

    /* create message to send */
    struct message msg1;
    msg1.msg_type = 1;
    strcpy(msg1.msg_text, "Hello Hehe");

    /* send message */
    if (msgsnd(msg_id, "Khanh DB", strlen(msg1.msg_text) + 1, 0) == -1)
    {
        perror("msgsnd");
        return -3;
    }

    // getchar();
    return 0;
}
#elif CASE == 1
#include <stdio.h>
#include <mqueue.h>
#include <string.h>

int status; /* stored the return value of function to check error */

int main(int argc, char *argv[])
{
    struct mq_attr attr, *get_attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 256;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open("/hihi", O_CREAT | O_RDWR, 0666, &attr);

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

    /*send message */
    char msg[] = "Hello HiHi";

    status = mq_send(mq, msg, strlen(msg) + 1, 100);

    if (status == -1)
    {
        perror("mq_send");
        return -2;
    }
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
unsigned short sem_val[3];

void handle(int sig)
{
    /* detach */
    if (shmdt(ptr) == -1)
    {
        perror("shmdt");
    }

    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    exit(123);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle); /* signal handle Ctrl + C */

    /* generate the key */
    // key_t key = ftok("/tmp/myshare", 'A');

    /* create the shared memory segment */
    shm_id = shmget(SHARED_MEMORY_KEY, SEGMENT_SIZE, 0666 | IPC_CREAT | IPC_EXCL);

    if (shm_id == -1) /* check error */
    {
        perror("shmget");
    }
    else
    {
        printf("Shared memory id: %d\n", shm_id);

        /* attach shared memory */
        ptr = shmat(shm_id, NULL, 0);
        if ((void *)ptr == (void *)-1) /* check error */
        {
            perror("shmat");
            return -1;
        }
        printf("Shared memory address: %p\n", ptr);

        /* init the first value of shared memory */
        ptr->money = 0;
    }

    /* create semaphore */
    sem_id = semget(SEMAPHORE_KEY, 3, IPC_CREAT | 0666);

    if (sem_id == -1) /* already existed semaphore */
    {
        perror("semget");
    }
    else
    {
        printf("Semaphore id: %d\n", sem_id);

        /* semaphore control, set all value of sem is 0 */
        sem_val[0] = 0;
        sem_val[1] = 0;
        sem_val[2] = 0;

        if (semctl(sem_id, 0, SETALL, sem_val) == -1)
        {
            perror("semctl");
            return -1;
        }
    }

    /* create semaphore operation */
    sem_opr[0].sem_flg = 0;
    sem_opr[0].sem_num = 0; /* sem[0] : client */
    sem_opr[0].sem_op = 1;

    sem_opr[1].sem_flg = 0;
    sem_opr[1].sem_num = 1; /* sem[1] : server */
    sem_opr[1].sem_op = -1;

    /* loop */
    while (1)
    {
        ptr->money += 10;

        if (ptr->money >= 100)
        {
            strcpy(ptr->text, "server put in 100 money.");
            semop(sem_id, &sem_opr[0], 1);
            // printf("server add %d money.\n", ptr->money);
            semop(sem_id, &sem_opr[1], 1);
            printf("server received: %s\n", ptr->text);
        }
        sleep(1);
    }

    return 0;
}
#elif CASE == 3
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
#include <pthread.h>
#include <signal.h>

char buffer[512]; /* buffer save data received */
char msg[512];    /* data to send */

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
    /* the loop comunication */
    while (1)
    {
        /* get input message from keyboard */
        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            printf("ERROR: get input from keyboard.\n");
            return -1;
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
    
    /* create fifo if it does not exist */
    mkfifo("/tmp/fifo_A_to_B", 0666);
    mkfifo("/tmp/fifo_B_to_A", 0666);

    /* open fifo */
    fd_send = open("/tmp/fifo_A_to_B", O_WRONLY);
    fd_read = open("/tmp/fifo_B_to_A", O_RDONLY);

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

    printf("server started.\n");

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