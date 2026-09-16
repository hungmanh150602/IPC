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