#ifndef COMM_FUN_H
#define COMM_FUN_H
#ifdef WIN32
#include <WinSock.h>
#else
#include <sys/socket.h>
#include <errno.h>
#endif


#ifdef WIN32
#define MY_SOCKET SOCKET
#else
#define MY_SOCKET int
#endif

extern int mRecv(MY_SOCKET sock,char * Buff,int Len);
extern int mSend(MY_SOCKET sock,const char * Buff,int Len);
#endif COMM_FUN_H