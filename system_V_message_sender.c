/*
Sender
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
    msg1.msg_type = 1;
    strcpy(msg1.msg_text, "Hello Hehe");

    /* send message */
    if(msgsnd(msg_id, "Khanh DB", strlen(msg1.msg_text) + 1, 0) == -1)
    {
        perror("msgsnd");
        return -3;
    }

    // getchar();
    return 0;
}