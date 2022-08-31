#ifndef MEDIALIB_H__
#define MEDIALIB_H__
#include <unistd.h>
#include <proto.h>
struct mlib_listentry_st
{
      chnid_t chnid;
      char *desc;  
};
int mlib_getchnlist(struct mlib_listentry_st **,int *);//将文件系统中的所有节目单信息放在这个节目单数组中，并记录读取到的频道总数
int mlib_freechnlist(struct mlib_listentry_st *);
ssize_t mlib_readchn(chnid_t,void *,size_t);
#endif