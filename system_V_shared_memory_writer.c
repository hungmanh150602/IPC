#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "type.h"

int ret; /* stored the return value of function to check error */

int main(int argc, char *argv[])
{
    /* generate the key */
    key_t key = ftok("/tmp/myshare", 'A');
    /* create the shared memory segment */
    int shm_id = shmget(key, SEGMENT_SIZE, 0666 | IPC_CREAT);

    if (shm_id == -1)
    {
        perror("shmget");
        return -1;
    }
    printf("Shared memory id: %d\n", shm_id);

    /* attach */
    void *ptr = shmat(shm_id, NULL, 0);

    char msg[] = "Hello from writer.";

    memcpy(ptr, msg, sizeof(msg));

    // *(int *)ptr = 23;

    printf("Shared memory address: %p\n", ptr);

    /* detach */
    if (shmdt(ptr) == -1)
    {
        perror("shmdt");
    }

    getchar();
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}