#pragma once
#include <iostream>
#include <binlog/export.h>
#include <binlog/CRowData.h>
#include <MysqlOperation.h>
#include <lock.h>
#define TABLE_XML_PATH "config/tableinfo.xml"
using namespace std;
#include <vector>
#include <json/json.h>
#include "defines.h"
#include "increment_export.h"
class CPreBinlog
{
public:
	CPreBinlog(void);
	~CPreBinlog(void);
	static DWORD ProcessBinlogThread(LPVOID lparam);
	static CPreBinlog* getInstance();
	BOOL StartProcessBinlog();
	BOOL InitConfig();
	BOOL FindTableName(const string tableName);
	string BuildSendData(P_HEAD_GENERATE_DATA pData);
	static CPreBinlog* m_instance;

private:
	HANDLE m_processThread;
	DWORD m_processThreadExitCode;
	vector<string> m_careTableName;

	string m_storehouse_name;
	string m_storehouse_id;
};

