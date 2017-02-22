#include "BinlogMonitor.h"
#include "ParseLogEntry.h"

CBinlogMonitor::CBinlogMonitor(void)
{
}


CBinlogMonitor::~CBinlogMonitor(void)
{
}

CBinlogMonitor::CBinlogMonitor(cchar *szDirPath) {
	m_dirPath = szDirPath;
	m_monitorThread = NULL;
	m_binlogIndexFileLen = 0;
}

#ifdef WIN32
DWORD WINAPI monitorThread(LPVOID lpParamter) {
	CBinlogMonitor *pBinlogMonitorInstance = (CBinlogMonitor*)lpParamter;
	HANDLE hChangeHandle=FindFirstChangeNotificationA(pBinlogMonitorInstance->m_dirPath.c_str(), false, 
		FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_FILE_NAME);
	if(hChangeHandle==INVALID_HANDLE_VALUE){
		return 0;
	}
	while(true) {
		DWORD dRv = WaitForSingleObject(hChangeHandle,INFINITE);
		if(dRv == WAIT_OBJECT_0) {
			if(FindNextChangeNotification(hChangeHandle) == TRUE){
				printf("These change a file name\n");
				CParseLogEntry *pInstance = CParseLogEntry::getInstance();
				if(pInstance->refreshFileCache(pBinlogMonitorInstance->getBinlogIndex().c_str()) == true)
					pInstance->parseBegin();
/*				
				CRowsEvent *pEvent = NULL;
				while((pEvent = pInstance->consumeRowEvent()) != NULL){
					for(int i = 0; i < pEvent->m_row_before_values.size(); i++)
						tx_row_value::row_value_printf(&(pEvent->m_row_before_values[i]), i);

					for(int i = 0; i < pEvent->m_row_after_values.size(); i++)
						tx_row_value::row_value_printf(&(pEvent->m_row_after_values[i]), i);
				}*/
			}
		}
	}
	FindCloseChangeNotification(hChangeHandle);
}
#endif



bool CBinlogMonitor::startMonitor() {
	if(initBinlogInfo() == false)
		return false;
#ifdef WIN32
	m_monitorThread = CreateThread(NULL, 0, monitorThread, this, 0, NULL);
	CloseHandle(m_monitorThread);
#else

#endif
	if(m_monitorThread == NULL)
		return false;
	return true;
}

bool CBinlogMonitor::initBinlogInfo() {
//	if((m_binlogIndexFileLen = getBinlogIndexSize()) == 0)
//		return false;
	if(getBinlogIndexFileName() == "")
		return false;
	return true;
}

uint32 CBinlogMonitor::getBinlogIndexSize() {
	string binlogIndexFile = m_dirPath + "//" + BINLOG_INDEX_FILE_NAME;
	FILE *fp = fopen(binlogIndexFile.c_str(), "rb");
	if(fp == NULL)
		return 0;
	fseek(fp, 0, SEEK_END);
	m_binlogIndexFileLen = ftell(fp);
	fclose(fp);
	return m_binlogIndexFileLen;
}

string CBinlogMonitor::getBinlogIndexFileName(){
	string binlogIndexFile = m_dirPath + "//" + BINLOG_INDEX_FILE_NAME;
	FILE *fp = fopen(binlogIndexFile.c_str(), "r");
	if(fp == NULL)
		return "";
	char szBuf[256];
	while(fgets(szBuf, 256, fp) != NULL);
	fseek(fp, 0, SEEK_END);
	m_binlogIndexFileLen = ftell(fp);
	fclose(fp);
	int len = strlen(szBuf);
	if(szBuf[len-1] == '\n')szBuf[len - 1]='\0';
	m_binlogIndexFileName = szBuf;
	return m_binlogIndexFileName;
}

string CBinlogMonitor::getBinlogIndex() {
	if(getBinlogIndexSize() == m_binlogIndexFileLen)
		return m_binlogIndexFileName;
	else
		return getBinlogIndexFileName();
}