#include "StdAfx.h"
#include "UpdateDbThread.h"
#include <BuildFinally.h>
#include <MysqlOperation.h>
#include <lock.h>
#include <LogData.h>
CUpdateDbThread g_instance_CUpdateDbThread;
CUpdateDbThread::CUpdateDbThread(void)
{
}


CUpdateDbThread::~CUpdateDbThread(void)
{
}

VOID CUpdateDbThread::ProductData(SOCKET_MESSAGE_HEADER header, const string& strData) {
	ENTRY_MUTEX(CUpdateDbThread);
	if(m_queue_data.size() >= ROWS_DATA_MAX_SIZE) {
		CONDITION_SLEEP(CUpdateDbThread, full);
	}
	else {
		UPDATE_DATA tmp;
		tmp.header_ = header;
		tmp.body_ = strData;
		m_queue_data.push_back(tmp);
	}
	CONDITION_WAKEUP(CUpdateDbThread, empty);
	LEAVE_MUTEX(CUpdateDbThread);
}


BOOL CUpdateDbThread::ConsumerData(UPDATE_DATA &data) {
	ENTRY_MUTEX(CUpdateDbThread);
	BOOL rv = TRUE;
	if(m_queue_data.size() == 0) {
		CONDITION_SLEEP(CUpdateDbThread, empty);
	}
	if(m_queue_data.size() > 0) {
		data = m_queue_data.front();
		m_queue_data.pop_front();
		rv = TRUE;
	}
	else {
		rv = FALSE;
	}
	
	CONDITION_WAKEUP(CUpdateDbThread, full);
	LEAVE_MUTEX(CUpdateDbThread);
	return rv;
}

BOOL CUpdateDbThread::Run() {
	m_handler_thread = CreateThread(NULL, 0, ThreadProcess, this, CREATE_SUSPENDED, &m_handler_thread_id);
	if(m_handler_thread == NULL)
		return FALSE;
	ResumeThread(m_handler_thread);
	return TRUE;
}

DWORD CUpdateDbThread::ThreadProcess(LPVOID lparam) {
	CUpdateDbThread* pInstance = (CUpdateDbThread*)lparam;
	UPDATE_DATA tmpData;
	BOOL rv = FALSE;
	string strData = "";
	while(true) {
		rv = pInstance->ConsumerData(tmpData);
		if(rv == FALSE) {
			continue;
		}
		try{
			CBuildFinally buildInstance(tmpData.body_);
			buildInstance.LoadJson();
			CMysqlOperation::getInstance()->execMutilSql(buildInstance.m_sql_select, buildInstance.m_sql_insert, buildInstance.m_sql_update);
			CLogData::getInstance()->WriteLogFormat("successData", tmpData.header_.message_id_, tmpData.body_.c_str(), NULL);
		}
		catch(ExceptionParsejson &e){
			CLogData::getInstance()->WriteLogFormat("failedData", tmpData.header_.message_id_, tmpData.body_.c_str(), NULL);
		}
		catch(ExceptionDatabase &e){
			CLogData::getInstance()->WriteLogFormat("failedData", tmpData.header_.message_id_, tmpData.body_.c_str(), NULL);
		}
	}
	return 0;
}