#ifndef COM_APP_H
#define COM_APP_H

struct mailbox
{
    //0: unqueued
    //1: queued
    unsigned char type;
    unsigned char msg_data_count;
    struct msg_data *msg_data_head;
    struct msg_data *msg_data_tail;
};

struct msg_data
{
    char buf[256];
    struct msg_data *next;
};

#endif  //ifndef COM_APP_H
