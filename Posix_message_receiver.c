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