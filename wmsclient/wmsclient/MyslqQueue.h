#pragma once
#include <iostream>
#include <vector>
#include <lock.h>
#include <mysql.h>
#include <MyException.h>
#include "increment_export.h"
#define TABLE_XML_PATH "config/tableinfo.xml"

#define ROWS_DATA_MAX_SIZE 1000
#define CONNCETC_DATA_BASE "Provider=SQLOLEDB.1;Password=123456;Persist Security Info=True;User ID=root;Initial Catalog=bjd02;Data Source=localhost"


using namespace std;
class CMysqlQueue {
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
	CMysqlQueue(void);
	virtual ~CMysqlQueue(void) throw();
	static CMysqlQueue* m_pInstance;
	static CMysqlQueue* getInstance();
	static DWORD doPingThread(LPVOID lparam);
	static void DestoryInstance();
	unsigned long getThreadId();
	void updateThreadId();
	void setConnectB(BOOL brv);
	int pingServer();
	return_val commitSql();
	return_val execOneSql(const string strSql);
	return_val getQueryRows(const string strSql, unsigned long long& rowLen);
	vector<string> getQueryData(const string strColum);
	return_val getQueryKeysData(const string strColum,vector<string> &vecColum);
	return_val getQueryData(const string strQuery,const string strColum,const string tableName);
	return_val getQueryData(const string strQuery,const string tableName,vector<vector<string>> &vecValues);
	return_val execMutilSql(const vector<string> sqlSelec,const vector<string> strColum,const vector<string> tableName);
	return_val execQuerySql(const string msgHeadKey,const string strQuery,const string strColum,const string tableName);
	return_val execQuerySql(const string strQuery,const string strColum,const string tableName,vector<string> &queueTable,vector<vector<string>> &vecKeys,vector<vector<vector<string>>> &vecValues);
	
	BOOL PutMessageQueue(const string msgHeadKey,const string tableName,const vector<string> vecKeys,vector<vector<string>> vecValues);
	BOOL PutMessageQueue(const string msgHeadKey, const vector<string> tableName,const vector<vector<string>> vecKeys,const vector<vector<vector<string>>> vecValues);
	BOOL PutOneMessageQueue(const string msgHeadKey,const long totalNum,const long theFirstFew,const string tableName,const vector<string> vecKeys,vector<vector<string>> vecValues);
	BOOL CMysqlQueue::PutOneMessageQueue(const string msgHeadKey, const vector<string> tableName,const vector<vector<string>> vecKeys,const vector<vector<vector<string>>> vecValues);
private:
	BOOL initDbConfig();
	BOOL connectDb();
	BOOL checkConnectAlive();
	VOID ProductData(P_GENERATE_DATA getDate);
	P_GENERATE_DATA ConsumerData();
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

	deque<P_GENERATE_DATA> m_queue_data;
};

