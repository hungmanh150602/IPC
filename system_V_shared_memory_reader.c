#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "type.h"

int ret; /* stored the return value of function to check error */

int main(int argc, char *argv[])
{
    /* generate the key */
    key_t key = ftok("/tmp/myshare", 'A');

    /* create the shared memory segment */
    int shm_id = shmget(SHARED_MEMORY_KEY, SEGMENT_SIZE, 0666 | IPC_CREAT);

    if (shm_id == -1)
    {
        perror("shmget");
        return -1;
    }
    printf("Shared memory id: %d\n", shm_id);

    /* attach */
    struct data *ptr = shmat(shm_id, NULL, 0);
    printf("Shared memory address: %p\n", ptr);

    printf("Value:\n");
    printf("x: %d\n", ptr->x);
    printf("y: %f\n", ptr->y);
    printf("c: %s\n", ptr->c);

    /* detach */
    if (shmdt(ptr) == -1)
    {
        perror("shmdt");
    }

    getchar();
    return 0;
}