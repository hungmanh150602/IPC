#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>
#include <sys/msg.h>

struct message
{
    long msg_type;
    char msg_text[128];
};

struct data
{
    float y;
    int x;
    char c[20];
};

void print_info(int key, int msg_id, const struct msqid_ds *ds,
                const char *buff_recv, const struct data *_data)
{
    printf("Key: %d\n", key);
    printf("message ID: %d\n", msg_id);

    if (ds != NULL)
    {
        printf("uid = %u\n", ds->msg_perm.uid);
        printf("gid = %u\n", ds->msg_perm.gid);
        printf("cuid = %u\n", ds->msg_perm.cgid);
        printf("cgid = %u\n", ds->msg_perm.cgid);
        printf("mode = %o\n", ds->msg_perm.mode);
        printf("key = %x\n", ds->msg_perm.__key);
        printf("num messages = %lu\n", ds->msg_qnum);
        printf("current byte = %lu\n", ds->__msg_cbytes);
    }

    if (buff_recv != NULL)
    {
        printf("message received: %s\n", buff_recv);
    }

    if (_data != NULL)
    {
        printf("%.4f\n", _data->y);
        printf("%d\n", _data->x);
        printf("%s\n", _data->c);
    }
    return;
}
#endif

/* Example send a data struct via Message Queue

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

    struct data mckd;
    mckd.y = 1.2;
    mckd.x = 3;
    strcpy(mckd.c, "HiHiHi");

    struct message msg1;
    // msg1.msg_type = 1;
    memcpy(msg1.msg_text, &mckd, sizeof(mckd));

    if (msgsnd(msg_id, &msg1, sizeof(msg1.msg_text), 0) == -1)
    {
        perror("msgsnd");
        return -3;
    }

    struct message receive;
    if (msgrcv(msg_id, &receive, sizeof(receive.msg_text), 0, 0) == -1)
    {
        perror("msgrcv");
        return -2;
    }

    struct data ccc;
    memcpy(&ccc, receive.msg_text, sizeof(ccc));

    struct msqid_ds ds;
    msgctl(msg_id, IPC_STAT, &ds);

    print_info(key, msg_id, &ds, NULL, &ccc);

    // getchar();
    return 0;
}
*/