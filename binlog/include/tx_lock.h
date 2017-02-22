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
#define CONDITION_INIT(pre, empty, full) InitializeCriticalSection(&pre##cs); \
											InitializeConditionVariable(&pre##cv_##empty); \
											InitializeConditionVariable(&pre##cv_##full);
							
#define ENTRY_MUTEX(pre) EnterCriticalSection(&pre##cs);
#define LEAVE_MUTEX(pre) LeaveCriticalSection(&pre##cs);
#define CONDITION_SLEEP(pre, what) SleepConditionVariableCS(&pre##cv_##what,&pre##cs,INFINITE);
#define CONDITION_WAKEUP(pre, what) WakeConditionVariable(&pre##cv_##what);
#define CONDITION_DESTORY(pre, empty, full) \
	do{                        \
		WakeAllConditionVariable(&pre##cv_##empty);\
		WakeAllConditionVariable(&pre##cv_##full);\
		Sleep(50);            \
		DeleteCriticalSection(&pre##cs); \
	}while(0);
#define CONDITION_WAIT(pre, what) \
	do{                     \
		EnterCriticalSection(&pre##cs); \
		SleepConditionVariableCS(&pre##cv_##what,&pre##cs,INFINITE); \
		LeaveCriticalSection(&pre##cs); \
	}while(0);
#define CONDITION_NOTIFY(pre, what) \
	do{                       \
		EnterCriticalSection(&pre##cs); \
		WakeConditionVariable(&pre##cv_##what); \
		LeaveCriticalSection(&pre##cs); \
	}while(0);
#else
#include <pthread.h>
#define MUTEX_VALUE pthread_mutex_t mutex;
#define MUTEX_INIT mutex = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK tx::Lock lock(mutex);
#define MUTEX_DESTORY(pre)
#endif
EXTERN_CONDITION_VALUE(CRowData, empty, full);
extern  MUTEX_VALUE(ParseLogEntry);
extern  void initMutex();
extern  void destoryMutex();

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