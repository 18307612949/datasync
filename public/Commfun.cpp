#include "Commfun.h"

int mRecv(MY_SOCKET sock,char * Buff,int Len) {
	int SumByte = 0;
	while(SumByte < Len ){
		int nRecv = recv(sock, Buff+SumByte, Len-SumByte, 0);
                if(nRecv < 0) {
                    return -2;
                }
		
#ifdef WIN32
		int errorV = WSAGetLastError();
		if(nRecv == SOCKET_ERROR){
			if(errorV == WSAEINTR){
				continue;
			}
			else if(errorV == WSAENOTSOCK){
				return -2;//自己已经主动关闭套接字
			}
			else{
				return -1;//接收失败
			}
		}
		else if(nRecv == 0){
			return -3;//对方关闭套接字
		}
#else

                
#endif
		SumByte += nRecv;
	}
	return SumByte;
}
int mSend(MY_SOCKET sock,const char * Buff,int Len) {
	int SumByte = 0;
	while(SumByte < Len ) {
		int nSend = send(sock, Buff+SumByte, Len-SumByte, 0);
                if(nSend < 0) {
                    return -2;
                }
#ifdef WIN32
		int errorV = WSAGetLastError();
		if(nSend == SOCKET_ERROR) {
			if(errorV == WSAEINTR) {
				continue;
			}
			else if(errorV == WSAENOTSOCK) {
				return -2;//自己已经主动关闭套接字
			}
			else {
				return -1;//发送失败
			}
		}
		else if(nSend == 0){
			return -3;//对方关闭套接字
		}

#else

#endif
		SumByte += nSend;
	}
	return SumByte;	
}
