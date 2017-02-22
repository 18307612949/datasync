#ifndef TX_LOCK_
#define TX_LOCK_
#ifdef WIN32
#include <Windows.h>
#define MUTEX_VALUE(pre) CRITICAL_SECTION pre##cs;
#define MUTEX_INIT(pre) InitializeCriticalSection(&pre##cs);
#define MUTEX_LOCK(pre) tx::Lock lock(pre##cs);
#define MUTEX_DESTORY(pre) DeleteCriticalSection(&pre##cs);

#define CONDITION_VALUE(pre, empty, full) CONDITION_VARIABLE pre##cv_##empty; \
											CONDITION_VARIABLE pre##cv_##full;\
											CRITICAL_SECTION pre##cs;
#define EXTERN_CONDITION_VALUE(pre, empty, full) extern CONDITION_VARIABLE pre##cv_##empty; \
											extern CONDITION_VARIABLE pre##cv_##full;\
											extern CRITICAL_SECTION pre##cs;
#define CONDITION_INIT(pre, empty, full)    \
	do{										\
		InitializeCriticalSection(&pre##cs); \
		InitializeConditionVariable(&pre##cv_##empty); \
		InitializeConditionVariable(&pre##cv_##full);	\
	}while(0);
#define CONDITION_DESTORY(pre, empty, full) \
	do{                        \
		WakeAllConditionVariable(&pre##cv_##empty);\
		WakeAllConditionVariable(&pre##cv_##full);\
		Sleep(50);            \
		DeleteCriticalSection(&pre##cs); \
	}while(0);
#define ENTRY_MUTEX(pre) EnterCriticalSection(&pre##cs);
#define LEAVE_MUTEX(pre) LeaveCriticalSection(&pre##cs);
#define CONDITION_SLEEP(pre, what) SleepConditionVariableCS(&pre##cv_##what,&pre##cs,INFINITE);
#define CONDITION_WAKEUP(pre, what) WakeConditionVariable(&pre##cv_##what);

#else
#include <pthread.h>
#define MUTEX_VALUE(pre) pthread_mutex_t pre##mutex;
#define MUTEX_INIT(pre) pre##mutex = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK(pre) tx::Lock lock(pre##mutex);
#define MUTEX_DESTORY(pre)

#define CONDITION_VALUE(pre, empty, full) pthread_cond_t pre##cond_##empty; \
											pthread_cond_t pre##cond_##full;\
											pthread_mutex_t pre##mutex;
#define EXTERN_CONDITION_VALUE(pre, empty, full) extern pthread_cond_t pre##cond_##empty;\
											extern pthread_cond_t pre##cond_##full;\
											extern pthread_mutex_t pre##mutex;
#define CONDITION_INIT(pre, empty, full)    \
	do{										\
		pre##mutex = PTHREAD_MUTEX_INITIALIZER; \
		pthread_cond_init(&pre##cond_##empty, NULL);\
		pthread_cond_init(&pre##cond_##full, NULL);	\
	}while(0);
#define CONDITION_DESTORY(pre, empty, full) \
	do{                        \
		pthread_cond_destroy(&pre##cond_##empty);\
		pthread_cond_destroy(&pre##cond_##full);\
	}while(0);

#define ENTRY_MUTEX(pre) pthread_mutex_lock(&pre##mutex);
#define LEAVE_MUTEX(pre) pthread_mutex_unlock(&pre##mutex);
#define CONDITION_SLEEP(pre, what) pthread_cond_wait(&pre##cond_##what,&pre##mutex);
#define CONDITION_WAKEUP(pre, what) pthread_cond_signal(&pre##cond_##what);
#endif


extern MUTEX_VALUE(LogErrorLock);
extern MUTEX_VALUE(LogDataLock);
#ifdef WIN32
extern MUTEX_VALUE(XMLWrite);
extern MUTEX_VALUE(ReadDBIncrement)
extern MUTEX_VALUE(MysqlOp);
extern MUTEX_VALUE(MysqlQueue)
extern MUTEX_VALUE(RefreshUI);
extern MUTEX_VALUE(ClientSocketServer);
extern MUTEX_VALUE(CPreBinlog);
EXTERN_CONDITION_VALUE(CUpdateDbThread, empty, full);
EXTERN_CONDITION_VALUE(CMysqlQueue, empty, full);
EXTERN_CONDITION_VALUE(IncrementDataLock,empty,full);
EXTERN_CONDITION_VALUE(CProcessMqRawData,empty,full);
#else
extern MUTEX_VALUE(SocketServer);
extern MUTEX_VALUE(SocketServerClientManager);
extern MUTEX_VALUE(AliMessageQueue);
extern MUTEX_VALUE(HandlerClientSocket);
EXTERN_CONDITION_VALUE(CProcessMqData, empty, full);
EXTERN_CONDITION_VALUE(CProcessMqRawData,empty,full);
#endif

extern void  initMutex();
extern void destoryMutex();
namespace tx{
	class Lock{
	
#ifdef WIN32
	private:
		CRITICAL_SECTION& m_cs;  
	public:  
		Lock(CRITICAL_SECTION& cs) : m_cs(cs){  
			EnterCriticalSection(&m_cs); 
		}  
		~Lock(){  
			LeaveCriticalSection(&m_cs);
		}  
#else
	private:
		pthread_mutex_t& m_mutex;
	public:
		Lock(pthread_mutex_t& mutex):m_mutex(mutex){
			pthread_mutex_lock(&m_mutex);
		}
		~Lock(){
			pthread_mutex_unlock(&m_mutex);
		}
#endif
	};
}

#endif