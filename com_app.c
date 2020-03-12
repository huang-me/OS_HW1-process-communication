#include "com_app.h"
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NETLINK_USER 31
#define MAX_PAYLOAD 1024  /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

void send_msg(int user_id,int to_id, char *message)
{

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid()%10000;                                          //id test
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;                                           //to_id
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid()%10000;                                            //id test
    nlh->nlmsg_flags = 0;

    char input[300] = "NULL" ;
    memset(input, 0, strlen(input));
    strcat(input,message);              //the index that passed to kernel
    strcpy(NLMSG_DATA(nlh), input);
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sendmsg(sock_fd,&msg,0);
    memset(nlh,0,NLMSG_SPACE(MAX_PAYLOAD));
    recvmsg(sock_fd, &msg, 0);
    printf("%s\n",NLMSG_DATA(nlh));
}

void main(int argc,char *argv[])
{
    int myid = atoi(argv[1]),to_id=0;
    char id_text[5],action[290],str[256],buffer[4096];
    strcpy(id_text,argv[1]);
    if(atoi(argv[1])&&(argc==3)&&(strcmp(argv[2],"queued")||strcmp(argv[2],"unqueued")))
    {

        strcpy(buffer,"Registration.id=0000");
        strncpy(&buffer[(20-strlen(id_text))],id_text,strlen(id_text));
        strcat(buffer,",type=");
        strcat(buffer,argv[2]);
        sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
        send_msg(myid,0,buffer);
        if(!strcmp(NLMSG_DATA(nlh),"Fail"))
            return;
        while(1)
        {
            scanf("%s",action);
            if(!strcmp(action,"Recv"))
            {
                strcat(action,".id=0000");
                strncpy(&action[12-strlen(id_text)],id_text,strlen(id_text));
                send_msg(myid,0,action);
            }
            else if(!strcmp(action,"Send"))
            {
                strcat(action,".id=0000");
                scanf("%s",id_text);
                strncpy(&action[12-strlen(id_text)],id_text,strlen(id_text));
                strcat(action,",str=");
                scanf("%255[^\n]s",str);
                if(scanf("%[^\n]s",buffer))
                {
                    printf("string too long!\nPlease send shorter string.\n");
                    continue;
                }
                strncat(action,&str[1],256);
                send_msg(myid,0,action);
            }
            else
            {
                printf("THE OPERATION IS NOT ACCEPTED!\n");
                break;
            }
        }
    }
    close(sock_fd);
}