#pragma once
#include <cstring>
#include <map>
#include <deque>
#include <tx_ctype.h>
#include "FileCache.h"
#include "LogEvent.h"
#include "tx_lock.h"
extern CRITICAL_SECTION cs;
class CParseLogEntry
{
public:
	CParseLogEntry(void);
	CParseLogEntry(cchar *filePath);
	~CParseLogEntry(void);
	static CParseLogEntry* getInstance();
	bool parseBegin();
	bool refreshFileCache(cchar *filePath);
	std::map<uint32, CTableMapEvent*>& getTableMapEvents(){return m_p_tablemaps;}
	static CParseLogEntry *m_pInstance;
	
private:
	bool checkHeader();
	bool parseEvents();
	bool parseOneEvent();
	CTableMapEvent* getTableMapEvent(uint32 tableId);
	void updateTableMap(CTableMapEvent *pMapEvent);
	void freeRowEvent(CRowsEvent* pRowEvent);

	FileCache m_fileCache;
	std::map<uint32, CTableMapEvent*> m_p_tablemaps;
	CQueryEvent *m_p_query_tmp;
	CTableMapEvent *m_p_map_tmp;
	uint32 m_binlog_current_file_ptr;
	std::string m_binlog_file_path;


	int m_index;
private:
	class CGarbo{
	public:
		~CGarbo(){
			if(CParseLogEntry::m_pInstance != NULL) {
				delete CParseLogEntry::m_pInstance;
				CParseLogEntry::m_pInstance = NULL;
			}
		}
	};
	static CGarbo garbo;
};

