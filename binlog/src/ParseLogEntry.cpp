#include "ParseLogEntry.h"
#include "tx_common.h"
#include "LogEvent.h"
#include "CRowData.h"
CParseLogEntry* CParseLogEntry::m_pInstance = NULL;
CParseLogEntry::CParseLogEntry(void) {
	m_p_tablemaps.erase(m_p_tablemaps.begin(), m_p_tablemaps.end());
	m_binlog_current_file_ptr = 0;
	m_index = 0;
	m_p_query_tmp = NULL;
	m_p_map_tmp = NULL;
}
CParseLogEntry* CParseLogEntry::getInstance() {
	if(m_pInstance == NULL) {
		MUTEX_LOCK(ParseLogEntry);
		if(m_pInstance == NULL) {
			m_pInstance = new CParseLogEntry();
		}
	}
	return m_pInstance;
}

CParseLogEntry::~CParseLogEntry(void) {
	std::map<uint32, CTableMapEvent*>::iterator it1;
	for(it1 = m_p_tablemaps.begin(); it1 != m_p_tablemaps.end(); it1++) {
		if(it1->second != NULL) {
			delete (CTableMapEvent*)(it1->second);
			it1->second = NULL;
		}
	}
	m_p_tablemaps.erase(m_p_tablemaps.begin(), m_p_tablemaps.end());
}
void CParseLogEntry::freeRowEvent(CRowsEvent* pRowEvent) {
	switch(pRowEvent->getGeneralTypeCode()) {
		case binary_log::UNKNOWN_EVENT:
			delete (CRowsEvent*)pRowEvent;
			break;
		case binary_log::DELETE_ROWS_EVENT:
			delete (CRowDeleteEvent*)pRowEvent;
			break;
		case binary_log::UPDATE_ROWS_EVENT:
			delete (CRowUpdateEvent*)pRowEvent;
			break;
		case binary_log::WRITE_ROWS_EVENT:
			delete (CRowWriteEvent*)pRowEvent;
			break;
		default:
			break;
	}
}

CParseLogEntry::CParseLogEntry(cchar *filePath) {
	m_fileCache.init(filePath);
	m_binlog_current_file_ptr = 0;
	m_p_tablemaps.erase(m_p_tablemaps.begin(), m_p_tablemaps.end());
}

bool CParseLogEntry::refreshFileCache(cchar *filePath) {
	return m_fileCache.init(filePath);
}
bool CParseLogEntry::parseBegin() {
	if(m_binlog_current_file_ptr == 0) {
		if(checkHeader() == false) {
			return false;
		}
	}
	else {
		if(m_fileCache.getFileSize() <= m_binlog_current_file_ptr)
			return true;
		m_fileCache.fileSeek(m_binlog_current_file_ptr,SEEK_SET);
	}
	if(parseEvents() == false)
		return false;
	m_binlog_current_file_ptr = m_fileCache.getFileSize();
	return true;
}
bool CParseLogEntry::checkHeader() {
	uchar *ucHeader;
	uchar *ucHeaderBuf;
	uchar *ucBuf;
	ucHeader = m_fileCache.readCache(BIN_LOG_HEADER_SIZE);
	if(ucHeader == NULL)
		return false;
	if(memcmp(ucHeader, BINLOG_MAGIC, BIN_LOG_HEADER_SIZE) != 0) {
		return false;
	}
	while(true) {
		
		if((ucHeaderBuf = m_fileCache.readCache(LOG_EVENT_HEADER_LEN)) == NULL)
			return false;

		uint eventSize = uint4korr(ucHeaderBuf+EVENT_LEN_OFFSET);
		uchar eventType = ucHeaderBuf[EVENT_TYPE_OFFSET];

		if(eventType == binary_log::START_EVENT_V3) {
			if(eventSize < (LOG_EVENT_MINIMAL_HEADER_LEN + binary_log::Binary_log_event::START_V3_HEADER_LEN)) {

			}
			m_fileCache.fileSeek(eventSize - LOG_EVENT_HEADER_LEN, SEEK_CUR);
		}
		else if(eventType == binary_log::FORMAT_DESCRIPTION_EVENT) {
			if((ucBuf = m_fileCache.readCache(eventSize, -LOG_EVENT_HEADER_LEN, SEEK_CUR, true)) == NULL) {
				return false;
			}
			global_description_event.setStream(ucBuf);
			if(global_description_event.loadEvent() != log_state::EVENT_SUCCESS)
				return false;
		}
		else if(eventType == binary_log::PREVIOUS_GTIDS_LOG_EVENT) {
			m_fileCache.fileSeek(eventSize - LOG_EVENT_HEADER_LEN, SEEK_CUR);
		}
		else if(eventType == binary_log::ROTATE_EVENT) {
			m_fileCache.fileSeek(eventSize - LOG_EVENT_HEADER_LEN, SEEK_CUR);
		}
		else {
			m_fileCache.fileSeek(-LOG_EVENT_HEADER_LEN, SEEK_CUR);
			break;
		}
	}
	return true;
}
bool CParseLogEntry::parseEvents() {
	do{
		if(parseOneEvent() == false){
			printf("parseOneEvent false\n");
			break;
		}
	}while(!m_fileCache.fileEof());
	
	/*
	std::map<uint32, CTableMapEvent*>::iterator it;
	for(it = m_p_tablemaps.begin(); it != m_p_tablemaps.end(); it++) {
		CTableMapEvent *pEvent = it->second;
		printf("table:%s[%d]\n", pEvent->getTableName().c_str(), pEvent->getTableId());
	}
	
	
	std::vector<CRowsEvent*>::iterator it1;
	for(int i = m_index; i<m_p_row_events.size(); i++) {
		CRowsEvent *pEvent = m_p_row_events[i];
		printf("---------------------[[%d]][[%s]]------------------\n",pEvent->getTableId(), getTableMapEvent(pEvent->getTableId())->getTableName().c_str());
		for(int i = 0; i < pEvent->m_row_before_values.size(); i++)
			tx_row_value::row_value_printf(&(pEvent->m_row_before_values[i]), i);

		for(int i = 0; i < pEvent->m_row_after_values.size(); i++)
			tx_row_value::row_value_printf(&(pEvent->m_row_after_values[i]), i);
	}
	m_index = m_p_row_events.size();
	*/
	return true;
}

bool CParseLogEntry::parseOneEvent() {
	uchar *ucHeader = NULL;
	uchar *cBuf = NULL;
	uint headSize = std::min<uint>(global_description_event.getEventHeaderLength(), LOG_EVENT_MINIMAL_HEADER_LEN);
	uint eventPos = m_fileCache.fileTell();
	if((ucHeader = m_fileCache.readCache(headSize)) == NULL) {
		printf("read header failed \n");
		return false;
	}
	uint32 eventSize = uint4korr(ucHeader + EVENT_LEN_OFFSET);
	if(eventSize < headSize) {
		printf("too small\n");
		return false;
	}
	cBuf = (uchar*)malloc((eventSize+1) * sizeof(uchar));
	cBuf[eventSize] = 0;
	memcpy(cBuf, ucHeader, headSize);
	if(m_fileCache.readCache(cBuf + headSize, eventSize - headSize) == false){
		printf("read error\n");
		return false;
	}

	if(eventSize < EVENT_LEN_OFFSET || eventSize != uint4korr(cBuf + EVENT_LEN_OFFSET)) {
		//Sanity check failed
		printf("Sanity check failed\n");
		goto errend;
	}
	uchar eventType = cBuf[EVENT_TYPE_OFFSET];
	if(eventType > binary_log::ENUM_END_EVENT || eventType == binary_log::UNKNOWN_EVENT) {
		//event not found
		printf("event not found\n");
		goto errend;
	}
	CRowsEvent *pRowEvent = NULL;
	//printf("type::%d\n", eventType);
	switch(eventType) {
		case binary_log::QUERY_EVENT:{
			if(m_p_query_tmp != NULL) {
				delete m_p_query_tmp;
				m_p_query_tmp = NULL;
			}
			m_p_query_tmp = new CQueryEvent(eventPos, cBuf);
			m_p_query_tmp->loadEvent();
			break;
		}
		case binary_log::LOAD_EVENT:
		case binary_log::NEW_LOAD_EVENT: {

			break;
		}
		case binary_log::TABLE_MAP_EVENT: {
			CTableMapEvent *pEvent = new CTableMapEvent(eventPos, cBuf);
			pEvent->loadEvent();
			m_p_map_tmp = pEvent;
			updateTableMap(pEvent);
			break;
		}
		case binary_log::WRITE_ROWS_EVENT_V1:
		case binary_log::WRITE_ROWS_EVENT: {
			pRowEvent = new CRowWriteEvent(eventPos, cBuf);
			pRowEvent->loadEvent();
			CTableMapEvent *pTableMapEvent = getTableMapEvent(pRowEvent->getTableId());
			pRowEvent->setTableMap(pTableMapEvent);
			pRowEvent->parseEvent();
			break;
		}
		case binary_log::UPDATE_ROWS_EVENT:
		case binary_log::UPDATE_ROWS_EVENT_V1:{
			pRowEvent = new CRowUpdateEvent(eventPos, cBuf);
			pRowEvent->loadEvent();
			CTableMapEvent *pTableMapEvent = getTableMapEvent(pRowEvent->getTableId());
			pRowEvent->setTableMap(pTableMapEvent);
			pRowEvent->parseEvent();
			break;
		}
		case binary_log::DELETE_ROWS_EVENT_V1:
		case binary_log::DELETE_ROWS_EVENT: {
			pRowEvent = new CRowDeleteEvent(eventPos, cBuf);
			pRowEvent->loadEvent();
			CTableMapEvent *pTableMapEvent = getTableMapEvent(pRowEvent->getTableId());
			pRowEvent->setTableMap(pTableMapEvent);
			pRowEvent->parseEvent();
			break;
		}
		case binary_log::STOP_EVENT: {
			m_binlog_current_file_ptr = 0;
			break;
		}
		default: {
			//printf("undetailtype:%d\n", eventType);
			break;
		}
	}
	if(pRowEvent != NULL) {
		productRowData(pRowEvent->createRowData(m_p_query_tmp->getThreadId(),
			m_p_map_tmp->getDbName().c_str(),
			m_p_map_tmp->getTableName().c_str()));
		freeRowEvent(pRowEvent);
	}
	if(cBuf != NULL) {
		free(cBuf);
	}
	return true;
errend:
	return false;
}

CTableMapEvent* CParseLogEntry::getTableMapEvent(uint32 tableId) {
	std::map<uint32, CTableMapEvent*>::iterator it = m_p_tablemaps.find(tableId);
	return it == m_p_tablemaps.end()?NULL:it->second;
}

void CParseLogEntry::updateTableMap(CTableMapEvent *pMapEvent) {
	uint32 tableId = pMapEvent->getTableId();
	map<uint32, CTableMapEvent*>::iterator it;
	it = m_p_tablemaps.find(tableId);
	if(it == m_p_tablemaps.end()) {
		m_p_tablemaps.insert(std::pair<uint32, CTableMapEvent*>(tableId, pMapEvent));
	}
	else {
		delete (CTableMapEvent*)(it->second);
		it->second = pMapEvent;
	}
}
