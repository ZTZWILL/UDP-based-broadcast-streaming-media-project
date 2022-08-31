#ifndef PROTO_H__
#define PROTO_H__
#include <site_type.h>
#define DEFAULT_MGROUP "224.2.2.2"
#define DEFAULT_RCVPORT "1998"
#define CHNNR 100
#define LISTCHNID 0
#define MINCHNID 1
#define MAXCHNID (MINCHNID+CHNNR-1)
#define MSG_CHANNEL_MAX (65536-20-8)
#define MAX_DATA (MSG_CHANNEL_MAX-sizeof(chnid_t))
#define MSG_LIST_MAX (65536-20-8)
#define MAX_ENTRY (MSG_LIST_MAX-sizeof(chnid_t))
struct msg_channel_st//传输的数据结构体
{
    chnid_t chnid; //当前包传输的数据属于的频道，自定义数据类型，八位无符号整形，可以表示0-255
    uint8_t data[1];
}__attribute__((packed));
struct msg_listentry_st//每个频道的描述信息
{
    chnid_t chnid;//频道号
    uint16_t len;//描述每个频道结构体的大小
    uint8_t desc[1];//频道描述
}__attribute__((packed));
struct msg_list_st   //节目单结构体
{
    chnid_t chnid;  //0，代表这个是节目单包
    struct msg_listentry_st entry[1];
}__attribute__((packed));
#endif