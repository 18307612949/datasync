#include "StdAfx.h"
#include <afx.h>
#include <stdio.h>
#include <stdlib.h>
#include "MysqlOperation.h"
#include "defines.h"
#include <LogData.h>

CMysqlOperation* CMysqlOperation::m_pInstance = NULL;

CMysqlOperation::CMysqlOperation(void){
	m_threadId = 0;
	m_bConfig = initDbConfig();
	if(m_bConfig == TRUE)
		m_bConnect = connectDb();
	else
		m_bConnect = FALSE;
}


CMysqlOperation::~CMysqlOperation(void) throw(){
}

DWORD CMysqlOperation::doPingThread(LPVOID lparam) {
	CMysqlOperation *pInstance = (CMysqlOperation*)lparam;
	while(true) {
		Sleep(20000);
		if(pInstance->pingServer() == 0)
			pInstance->updateThreadId();
		else
			pInstance->setConnectB(FALSE);
	}
	return 0;
}

CMysqlOperation* CMysqlOperation::getInstance() {
	if(CMysqlOperation::m_pInstance == NULL){
		MUTEX_LOCK(MysqlOp);
		if(CMysqlOperation::m_pInstance == NULL)
			CMysqlOperation::m_pInstance = new CMysqlOperation();
	}
	return CMysqlOperation::m_pInstance;
}

void CMysqlOperation::destoryInstance() {
	MUTEX_LOCK(MysqlOp);
	if(CMysqlOperation::m_pInstance != NULL) {
		delete CMysqlOperation::m_pInstance;
		CMysqlOperation::m_pInstance = NULL;
	}
}

BOOL CMysqlOperation::initDbConfig() {
	CFileFind finder;
	BOOL ifFind = finder.FindFile(_T(CONFIG_PATH));
	if(ifFind == FALSE)
		return FALSE;
	DWORD iRv;
	iRv = ::GetPrivateProfileStringA(DB_CONFIG_KEY, "Host","",m_host.GetBuffer(MAX_PATH),MAX_PATH,CONFIG_PATH);
	if(iRv == 0)return FALSE;
	::GetPrivateProfileStringA(DB_CONFIG_KEY, "UserName", "", m_username.GetBuffer(MAX_PATH), MAX_PATH, CONFIG_PATH);
	if(iRv == 0)return FALSE;
	::GetPrivateProfileStringA(DB_CONFIG_KEY, "Password", "", m_password.GetBuffer(MAX_PATH), MAX_PATH, CONFIG_PATH);
	if(iRv == 0)return FALSE;
	::GetPrivateProfileStringA(DB_CONFIG_KEY, "DatabaseName", "", m_database.GetBuffer(MAX_PATH), MAX_PATH, CONFIG_PATH);
	if(iRv == 0)return FALSE;
	return TRUE;
}

BOOL CMysqlOperation::connectDb(){
	mysql_init(&m_connection);
//	char cValue = 1;
//	mysql_options(&m_connection, MYSQL_OPT_RECONNECT, (char*)&cValue);
	if(mysql_real_connect(&m_connection, (LPCSTR)m_host, (LPCSTR)m_username, (LPCSTR)m_password, (LPCSTR)m_database, 0, NULL, 0) == NULL) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return FALSE;
	}
	if(mysql_set_character_set(&m_connection, "gbk") != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return FALSE;
	}
	if(mysql_autocommit(&m_connection, 0) != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return FALSE;
	}
	HANDLE thHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)doPingThread, this, 0, NULL);
	if(thHandle == NULL)
		return FALSE;
	CloseHandle(thHandle);
	return TRUE;
}

void CMysqlOperation::setConnectB(BOOL brv) {
	m_bConnect = brv;
} 
BOOL CMysqlOperation::checkConnectAlive() {
	if(m_bConnect == FALSE) {
		return FALSE;
	}
	return TRUE;
}

void CMysqlOperation::setError(){
	m_mysql_errorno = mysql_errno(&m_connection);
	m_mysql_errormsg = mysql_error(&m_connection);
	char szTmp[256];
	sprintf_s(szTmp, 256, "%d", m_mysql_errorno);
	m_mysql_error_format = "";
	m_mysql_error_format += "[code:";
	m_mysql_error_format += szTmp;
	m_mysql_error_format += "]";
	m_mysql_error_format += m_mysql_errormsg;
}

CMysqlOperation::return_val CMysqlOperation::execOneSql(const string strSql){
	if(checkConnectAlive() == FALSE)
		return mysql_dbDisconnection; 
	int iCode = 0;
	if((iCode = mysql_real_query(&m_connection, strSql.c_str(), strSql.length())) != 0) {
		setError();
		CLogData::getInstance()->WriteLog(strSql.c_str(), 1000);
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	return mysql_success;
}

CMysqlOperation::return_val CMysqlOperation::commitSql(){
	if(mysql_commit(&m_connection) != 0){
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbError;
	}
	return mysql_success;
}

CMysqlOperation::return_val CMysqlOperation::getQueryRows(const string strSql, unsigned long long& rowLen){
	if(checkConnectAlive() == FALSE)
		return mysql_dbDisconnection;
	MYSQL_RES *mysqlRes = NULL;
	if(mysql_real_query(&m_connection, strSql.c_str(), strSql.length()) != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	if((mysqlRes = mysql_store_result(&m_connection)) == NULL) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	rowLen = mysql_num_rows(mysqlRes);
	return mysql_success;
}

CMysqlOperation::return_val CMysqlOperation::execMutilSql(const vector<string>& sqlSelect, const vector<string>& sqlInsert, const vector<string>& sqlUpdate) {
	MUTEX_LOCK(MysqlOp);
	int sqlSize = sqlSelect.size();
	unsigned long long rowNum = 0;
	for(int i = 0; i < sqlSize; i++) {
		if(getQueryRows(sqlSelect[i], rowNum) != mysql_success) {
			goto error;
		}
		if(rowNum > 0){
			if(execOneSql(sqlUpdate[i]) != mysql_success) {
				goto error;
			}
		}
		else {
			if(execOneSql(sqlInsert[i]) != mysql_success) {
				goto error;
			}
		}
	}
	mysql_commit(&m_connection);
	return mysql_success;
error:
	setError();
	THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
	mysql_rollback(&m_connection);
	return mysql_dbMuilFailed;
}

unsigned long CMysqlOperation::getThreadId() {
	return m_threadId;
}

void CMysqlOperation::updateThreadId(){
	m_threadId = mysql_thread_id(&m_connection);
}

int CMysqlOperation::pingServer(){
	MUTEX_LOCK(MysqlOp);
	int iRv = mysql_ping(&m_connection);
	return iRv;
}