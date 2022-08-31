#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <net/if.h>
#include <proto.h>
#include "client.h"
/*
-M --mgroup  指定多播组
-P --port   指定接受端口
-p --player  指定播放器
-H -- help  显示帮助
*/
struct client_conf_st client_conf={.rcvport=DEFAULT_RCVPORT,.mgroup=DEFAULT_MGROUP,.player_cmd=DEFAULT_PLAYERCMD};//定义为默认值
void printhelp(void){
    printf("-P --port      指定接受端口\n");
    printf("-M --mgroup    指定多播组\n");
    printf("-p --player    指定解码器\n");
    printf("-H --help      显示帮助\n");
}
static ssize_t writen(int fd,const char *buf,size_t len){//写入指定数量的字符到fd中
    int ret;
    int pos;
    pos=0;
    while(len>0){
        ret=write(fd,buf+pos,len);
        if(ret<0){
            if(errno == EINTR){
                continue;
            }
            perror("write()");
            return -1;
        }
        len-=ret;
        pos+=ret;
    }
    return pos;   
}

int main(int argc,char **argv)
{
    /*
    初始化的级别：逐渐增加
    默认值，配置文件，环境变量，命令行参数
    */
    
    int c;
    int index=0;
    struct option argarr[]={{"port",1,NULL,'P'},{"mgroup",1,NULL,'M'},{"palyer",1,NULL,'p'},{"help",0,NULL,'H'},{NULL,0,NULL,0}};   
    while(1){
        c=getopt_long(argc,argv,"P:M:p:H",argarr,&index);
        if(c<0){
            break;
        }
        switch(c){
            case 'P':
                client_conf.rcvport=optarg;
                break;
            case 'M':
                client_conf.mgroup=optarg;
                break;
            case 'p':
                client_conf.player_cmd=optarg;
                break;
            case 'H':
                printhelp();
                exit(0);
                break;
            default:
                abort();
                break;
        }
    }
    int sd;
    sd=socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0){
        perror("socket()");
        exit(1);
    }
    //设置加入多播组
    struct ip_mreqn mreq;
    inet_pton(AF_INET,client_conf.mgroup,&mreq.imr_multiaddr);
    inet_pton(AF_INET,"0.0.0.0",&mreq.imr_address);
    mreq.imr_ifindex=if_nametoindex("eth0");
    if(setsockopt(sd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0){
        perror("setsockopt()");
        exit(1);
    }
    int val=1;
    if(setsockopt(sd,IPPROTO_IP,IP_MULTICAST_LOOP,&val,sizeof(val))<0){
        perror("seysockopt()");
        exit(1);
    }
    //绑定地址
    struct sockaddr_in laddr;
    laddr.sin_family=AF_INET;
    laddr.sin_port=htons(atoi(client_conf.rcvport));
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);
    if(bind(sd,(void *)&laddr,sizeof(laddr))<0){
        perror("bind()");
        exit(1);
    }
    //创建管道
    int pd[2];
    if(pipe(pd)<0){
        perror("pipe()");
        exit(1);
    }
    //创建子进程
    int pid;
    pid=fork();
    if(pid<0){
        perror("fork()");
        exit(1);
    }
    if(pid==0){//子进程负责调用解码器
        close(sd);
        close(pd[1]);
        dup2(pd[0],0);
        if(pd[0]>0){
            close(pd[0]);
        }
        execl("/bin/sh","sh","-c",client_conf.player_cmd,NULL);
        perror("execl()");
        exit(1);
    }
    //父进程从网络上收取数据包向子进程发送
    //收节目单
    struct  msg_list_st *msg_list;
    msg_list = malloc(MSG_LIST_MAX);
    if(msg_list==NULL){
        perror("malloc()");
        exit(1);
    }
    struct sockaddr_in serveraddr;
    socklen_t serveraddr_len; 
    serveraddr_len = sizeof(serveraddr);
    int len;
    while(1){
        len=recvfrom(sd,msg_list,MSG_LIST_MAX,0,(void*)&serveraddr,&serveraddr_len);
        if(len<sizeof(struct msg_list_st)){
            fprintf(stderr,"message size is  too small.\n");
        }
        if(msg_list->chnid!=LISTCHNID){
            fprintf(stderr, "current chnid:%d.\n", msg_list->chnid);
            fprintf(stderr,"chnid is not match.\n");
            continue;
        }
        break;
    } 
    /*打印节目单并提示用户选择频道*/
    struct msg_listentry_st*pos;
    for(pos=msg_list->entry;(char*)pos<(((char*)msg_list)+len);pos=(void*)(((char*)pos)+ntohs(pos->len))){//打印每一个频道的描述
        printf("channel %d : %s\n",pos->chnid,pos->desc);
    }
    int chosenid;
    int ret=0;
    //free(msg_list);//释放自己申请的内存
    puts("Please enter your choice:");
    while(ret<1){
        ret=scanf("%d",&chosenid);
        //if(ret!=0){
            //exit(1);
        //}
    }
    puts("Please enjoy");
    //接收一般数据包
    struct msg_channel_st *msg_channel;
    msg_channel=malloc(MSG_CHANNEL_MAX);
    if(msg_channel==NULL){
        perror("malloc()");
        exit(1);
    }
    fprintf(stdout,"chosenid= %d\n",ret);
    struct sockaddr_in raddr;
    socklen_t raddr_len;
    raddr_len=sizeof(raddr);
    while(1){
        len=recvfrom(sd,msg_channel,MSG_CHANNEL_MAX,0,(void*)&raddr,&raddr_len);
        if(raddr.sin_addr.s_addr!=serveraddr.sin_addr.s_addr||raddr.sin_port!=serveraddr.sin_port){
            fprintf(stderr,"Ignore:address not match.\n");
            continue;
        }
        if(len<sizeof(struct msg_channel_st)){
            fprintf(stderr,"Ignore:message too small.\n");
            continue;
        }
        if(msg_channel->chnid==chosenid){//只有当完整接受到一个数据包时才会将用户选择的频道数据写入管道
            fprintf(stdout,"accepted msg:%d recieved.\n",msg_channel->chnid);
            if(writen(pd[1],msg_channel->data,len-sizeof(chnid_t))<0){
                exit(1);
            }
        }
    }
    free(msg_channel);
    close(sd);

    exit(0);
}