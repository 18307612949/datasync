#pragma once
#include "tx_ctype.h"
#include <iostream>

#ifdef WIN32
#include <Windows.h>
#include <conio.h>
#endif
using namespace std;
#define BINLOG_INDEX_FILE_NAME "logbin.index"
class DLL_SAMPLE_API CBinlogMonitor
{
public:
	CBinlogMonitor(void);
	CBinlogMonitor(cchar *szDirPath);
	~CBinlogMonitor(void);

	bool startMonitor();
	string getBinlogIndex();
	string m_dirPath;

private:
	bool initBinlogInfo(); //��ȡbinlogindex�ļ���С
	uint32 getBinlogIndexSize();
	string getBinlogIndexFileName();
	uint32 m_binlogIndexFileLen;
	string m_binlogIndexFileName;
	HANDLE m_monitorThread;

};

