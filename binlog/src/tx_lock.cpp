#include "tx_lock.h"
MUTEX_VALUE(ParseLogEntry);
CONDITION_VALUE(CRowData, empty, full);

void  initMutex(){
	MUTEX_INIT(ParseLogEntry);
	CONDITION_INIT(CRowData, empty, full);
}

void  destoryMutex(){
	MUTEX_DESTORY(ParseLogEntry);
	CONDITION_DESTORY(CRowData, empty, full);
}