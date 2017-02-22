#include <CRowData.h>
#include <tx_lock.h>
#define ROWS_DATA_MAX_SIZE 256

static deque<CRowData*> g_rowDatas;
bool productRowData(CRowData *data) {
	ENTRY_MUTEX(CRowData);
	if(g_rowDatas.size() >= ROWS_DATA_MAX_SIZE) {
		CONDITION_SLEEP(CRowData, full);
	}
	else {
		g_rowDatas.push_back(data);
	}
	CONDITION_WAKEUP(CRowData, empty);
	LEAVE_MUTEX(CRowData);
	return true;
}

DLL_SAMPLE_API CRowData *consumerRowData() {
	CRowData *pData = NULL;
	ENTRY_MUTEX(CRowData);
	if(g_rowDatas.size() == 0) {
		CONDITION_SLEEP(CRowData, empty);
	}
	
	pData = g_rowDatas.front();
	g_rowDatas.pop_front();
	CONDITION_WAKEUP(CRowData, full);
	LEAVE_MUTEX(CRowData);
	return pData;
}