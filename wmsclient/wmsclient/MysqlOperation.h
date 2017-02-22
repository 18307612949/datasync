#pragma once
#include <iostream>
#include <vector>
#include <lock.h>
#include <mysql.h>
#include <MyException.h>
using namespace std;
class CMysqlOperation {
public:
	enum return_val{
		mysql_success = -1000,
		mysql_dbError = -1001,
		mysql_dbDisconnection = -1002,
		mysql_dbExecFailed = -1003,
		mysql_dbMuilFailed = -1004,
		mysql_noData = -1005
	};
public:
	CMysqlOperation(void);
	virtual ~CMysqlOperation(void) throw();
	static CMysqlOperation* m_pInstance;
	static CMysqlOperation* getInstance();
	static void destoryInstance();
	static DWORD doPingThread(LPVOID lparam);
	unsigned long getThreadId();
	void updateThreadId();
	void setConnectB(BOOL brv);
	int pingServer();
	return_val commitSql();
	return_val execOneSql(const string strSql);
	return_val getQueryRows(const string strSql, unsigned long long& rowLen);
	return_val execMutilSql(const vector<string>& sqlSelect, const vector<string>& sqlInsert, const vector<string>& sqlUpdate);
private:
	BOOL initDbConfig();
	BOOL connectDb();
	BOOL checkConnectAlive();
	void setError();
	CStringA m_host;
	CStringA m_database;
	CStringA m_username;
	CStringA m_password;
	BOOL m_bConfig;
	BOOL m_bConnect;
	unsigned long m_threadId;
	unsigned int m_mysql_errorno;
	string m_mysql_errormsg;
	string m_mysql_error_format;

	MYSQL m_connection;


};

