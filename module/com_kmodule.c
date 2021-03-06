#include "com_kmodule.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#define NETLINK_USER 31

struct sock *nl_sk = NULL;
struct mailbox mail[1001];
struct nlmsghdr *nlh;
struct sk_buff *skb_out;
int res,mypid,account[1000]= {0},acc_num=0,i,pid;
char msg[1000];
char todo[13],id_text[6],str[500],*id_end;
int to_id,a=0;
struct msg_data *msg_data,*temp;
char as[200];
int change[10001];



static void hello_nl_recv_msg(struct sk_buff *skb)
{

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    nlh=(struct nlmsghdr*)skb->data;            //get msg from the application
    strcpy(msg, (char *)nlmsg_data(nlh));
    printk("%s",msg);
    pid = nlh->nlmsg_pid; /*pid of sending process */
    printk("%d",pid);
    strncpy(todo,msg,12);
    if(!strcmp(todo,"Registration"))         //registration
    {
        strncpy(id_text,&msg[16],4);
        sscanf(id_text,"%d",&mypid);
        change[pid]=mypid;
        printk("%d",mypid);
        strcpy(str,&msg[26]);
        for(i=0; i<acc_num; i++)
        {
            if(account[i]==mypid)
            {
                memset(msg,0,1000);
                strcpy(msg,"Fail");                 //return msg from kernel
                goto ack;
            }
        }
        account[acc_num++]=mypid;
        if(!strcmp(str,"queued"))
        {
            mail[mypid].type = 1;
            mail[mypid].msg_data_count = 0;
            mail[mypid].msg_data_head=mail[mypid].msg_data_tail=(struct msg_data*)kmalloc(sizeof(struct msg_data),GFP_KERNEL);
            mail[mypid].msg_data_head->next = (struct msg_data*)kmalloc(sizeof(struct msg_data),GFP_KERNEL);
            mail[mypid].msg_data_head->next->next = (struct msg_data*)kmalloc(sizeof(struct msg_data),GFP_KERNEL);
            mail[mypid].msg_data_head->next->next->next = mail[mypid].msg_data_head;
            mail[mypid].msg_data_tail->next = mail[mypid].msg_data_head->next;
        }
        else
        {
            mail[mypid].msg_data_head=mail[mypid].msg_data_tail=(struct msg_data*)kmalloc(sizeof(struct msg_data),GFP_KERNEL);
            mail[mypid].type = 0;
            mail[mypid].msg_data_count = 0;
        }
        memset(msg,0,1000);                //return msg from kernel
        strcpy(msg,"Success");
        printk("%c%d",mail[mypid].type,mail[mypid].type);
    }
    else
    {
        strncpy(id_text,&msg[8],4);
        sscanf(id_text,"%d",&to_id);
        memset(todo,0,12);
        strncpy(todo,msg,4);
        if(!strcmp(todo,"Recv"))                //receive
        {
            if(mail[change[pid]].type==0)               //unqueued
            {
                if(mail[change[pid]].msg_data_count==0)
                {
                    memset(msg,0,sizeof(msg));
                    strcpy(msg,"Fail");
                }
                else
                {
                    memset(msg,0,sizeof(msg));
                    strcpy(msg,mail[change[pid]].msg_data_head->buf);
                }
            }
            else                                                 //queued
            {
                if(mail[change[pid]].msg_data_count==0)
                {
                    memset(msg,0,sizeof(msg));
                    strcpy(msg,"Fail");
                }
                else
                {
                    memset(msg,0,sizeof(msg));
                    strcpy(msg,mail[change[pid]].msg_data_head->buf);
                    strcpy(mail[change[pid]].msg_data_head->buf,"");
                    mail[change[pid]].msg_data_count--;
                    mail[change[pid]].msg_data_head = mail[change[pid]].msg_data_head->next;
                }
            }
        }
        else if(!strcmp(todo,"Send"))                                    //send
        {
            strncpy(todo,&msg[8],4);
            sscanf(todo,"%d",&to_id);
            printk("%d",to_id);
            for(i=0; i<acc_num; i++)
            {
                if(account[i]==to_id)
                    a++;
            }
            if(a==0)
            {
                memset(msg,0,sizeof(msg));
                strcpy(msg,"Fail");
                goto ack;
            }
            memset(str,0,sizeof(str));
            strncpy(str,&msg[17],255);
            if(strlen(str)>255)                 //test if the string lenth is too long
            {
                memset(msg,0,sizeof(msg));
                strcpy(msg,"Fail");
                goto ack;
            }
            if(mail[to_id].type==0)                     //unqueued
            {
                strcpy(mail[to_id].msg_data_head->buf,str);
                mail[to_id].msg_data_count=1;
                printk("%d",mail[to_id].msg_data_count);
                memset(msg,0,sizeof(msg));
                strcpy(msg,"Success");
            }
            else                                          //queued
            {
                if(mail[to_id].msg_data_count==3)
                {
                    memset(msg,0,sizeof(msg));
                    strcpy(msg,"Fail");
                    goto ack;
                }
                else
                {
                    strcpy(mail[to_id].msg_data_tail->buf,str);
                    mail[to_id].msg_data_tail = mail[to_id].msg_data_tail->next;
                    mail[to_id].msg_data_count++;
                    memset(msg,0,sizeof(msg));
                    strcpy(msg,"Success");
                }
            }
        }
    }




ack:
    skb_out = nlmsg_new(strlen(msg),0);
    if(!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    nlh=nlmsg_put(skb_out,0,1,NLMSG_DONE,strlen(msg),0);

    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strcpy(nlmsg_data(nlh),msg);

    res=nlmsg_unicast(nl_sk,skb_out,pid);
    printk(KERN_INFO "%s\n",(char*)NLMSG_DATA((struct nlmsghdr*)skb_out->data));

    if(res<0)
        printk(KERN_INFO "Error while sending bak to user\n");
}




static int __init com_kmodule_init(void)
{
    printk("Entering: %s\n",__FUNCTION__);
    struct netlink_kernel_cfg cfg =
    {
        .input = hello_nl_recv_msg,
    };
    nl_sk=netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if(!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit com_kmodule_exit(void)
{
    printk(KERN_INFO "Exit module. Bye~\n");
    netlink_kernel_release(nl_sk);
}

module_init(com_kmodule_init);
module_exit(com_kmodule_exit);
