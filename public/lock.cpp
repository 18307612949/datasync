#include "lock.h"
#ifdef WIN32
MUTEX_VALUE(MysqlQueue)
MUTEX_VALUE(MysqlOp);
MUTEX_VALUE(RefreshUI);
MUTEX_VALUE(XMLWrite);
MUTEX_VALUE(ReadDBIncrement);
MUTEX_VALUE(ClientSocketServer);
MUTEX_VALUE(CPreBinlog);
CONDITION_VALUE(CUpdateDbThread, empty, full);
CONDITION_VALUE(CMysqlQueue, empty, full);
CONDITION_VALUE(IncrementDataLock,empty,full);
CONDITION_VALUE(CProcessMqRawData,empty,full);
#else
MUTEX_VALUE(SocketServer);
MUTEX_VALUE(SocketServerClientManager);
MUTEX_VALUE(AliMessageQueue);
MUTEX_VALUE(HandlerClientSocket);
CONDITION_VALUE(CProcessMqData, empty, full);
CONDITION_VALUE(CProcessMqRawData, empty, full);
CONDITION_VALUE(IncrementDataLock,empty,full);
#endif
MUTEX_VALUE(LogErrorLock);
MUTEX_VALUE(LogDataLock);
void  initMutex(){
	MUTEX_INIT(LogErrorLock);
	MUTEX_INIT(LogDataLock);
#ifdef WIN32
	MUTEX_INIT(MysqlOp);
	MUTEX_INIT(MysqlQueue);
	MUTEX_INIT(RefreshUI);
	MUTEX_INIT(XMLWrite);
	MUTEX_INIT(ReadDBIncrement);
	MUTEX_INIT(ClientSocketServer);
	MUTEX_INIT(CPreBinlog);
	CONDITION_INIT(CUpdateDbThread, empty, full);
	CONDITION_INIT(CMysqlQueue, empty, full);
	CONDITION_INIT(IncrementDataLock,empty,full);
	CONDITION_VALUE(CProcessMqRawData,empty,full);
#else
	MUTEX_INIT(SocketServer);
                  MUTEX_INIT(SocketServerClientManager);
                  MUTEX_INIT(AliMessageQueue);
                  CONDITION_INIT(CProcessMqData, empty, full);
#endif
}

void destoryMutex(){
	MUTEX_DESTORY(LogErrorLock);
	MUTEX_DESTORY(LogDataLock);
#ifdef WIN32
	MUTEX_DESTORY(MysqlOp);
	MUTEX_DESTORY(MysqlQueue);
	MUTEX_DESTORY(RefreshUI);
	MUTEX_DESTORY(XMLWrite);
	MUTEX_DESTORY(ReadDBIncrement);
	MUTEX_DESTORY(ClientSocketServer);
	MUTEX_DESTORY(CPreBinlog);
	CONDITION_DESTORY(CUpdateDbThread, empty, full);
	CONDITION_DESTORY(CMysqlQueue, empty, full);
	CONDITION_DESTORY(IncrementDataLock,empty,full);
	CONDITION_VALUE(CProcessMqRawData,empty,full);
#else
                  MUTEX_DESTORY(SocketServer);
                  MUTEX_DESTORY(SocketServerClientManager);
                  MUTEX_DESTORY(AliMessageQueue);
                  CONDITION_DESTORY(CProcessMqData, empty, full);
                  CONDITION_DESTORY(IncrementDataLock,empty,full);
                  CONDITION_VALUE(CProcessMqRawData,empty,full);
#endif
}
