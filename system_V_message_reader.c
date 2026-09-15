/*
Reader
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

    /* receive message */
    char buffer[100];
    struct message receive;
    if(msgrcv(msg_id, buffer, sizeof(buffer), 0, 0) == -1)
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