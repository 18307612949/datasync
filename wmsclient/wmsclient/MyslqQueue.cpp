#include "StdAfx.h"
#include <afx.h>
#include <stdio.h>
#include <stdlib.h>
#include "MyslqQueue.h"
#include "defines.h"
#include <LogData.h>
#include <MD5.h>
CMysqlQueue* CMysqlQueue::m_pInstance = NULL;

CMysqlQueue::CMysqlQueue(void){
	m_threadId = 0;
	m_queue_data.clear();
	m_bConfig = initDbConfig();
	if(m_bConfig == TRUE)
		m_bConnect = connectDb();
	else
		m_bConnect = FALSE;
}


CMysqlQueue::~CMysqlQueue(void) throw(){
}

DWORD CMysqlQueue::doPingThread(LPVOID lparam) {
	CMysqlQueue *pInstance = (CMysqlQueue*)lparam;
	while(true) {
		Sleep(20000);
		if(pInstance->pingServer() == 0)
			pInstance->updateThreadId();
		else
			pInstance->setConnectB(FALSE);
	}
	return 0;
}

CMysqlQueue* CMysqlQueue::getInstance() {
	if(CMysqlQueue::m_pInstance == NULL){
		MUTEX_LOCK(MysqlQueue);
		if(CMysqlQueue::m_pInstance == NULL)
			CMysqlQueue::m_pInstance = new CMysqlQueue();
	}
	return CMysqlQueue::m_pInstance;
}

void CMysqlQueue::DestoryInstance() {
	if(CMysqlQueue::m_pInstance != NULL) {
		delete CMysqlQueue::m_pInstance;
	}
}

BOOL CMysqlQueue::initDbConfig() {
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

BOOL CMysqlQueue::connectDb(){
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
	/*HANDLE thHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)doPingThread, this, 0, NULL);
	if(thHandle == NULL)
		return FALSE;
	CloseHandle(thHandle);*/
	return TRUE;
}

void CMysqlQueue::setConnectB(BOOL brv) {
	m_bConnect = brv;
} 
BOOL CMysqlQueue::checkConnectAlive() {
	if(m_bConnect == FALSE) {
		return FALSE;
	}
	return TRUE;
}

void CMysqlQueue::setError(){
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

CMysqlQueue::return_val CMysqlQueue::execOneSql(const string strSql){
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

CMysqlQueue::return_val CMysqlQueue::commitSql(){
	if(mysql_commit(&m_connection) != 0){
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbError;
	}
	return mysql_success;
}

CMysqlQueue::return_val CMysqlQueue::getQueryRows(const string strSql, unsigned long long& rowLen){
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

vector<string> CMysqlQueue::getQueryData(const string strColum){
	vector<string> vecTemp;
	vecTemp.clear();
	if(checkConnectAlive() == FALSE)
		return vecTemp;
	MYSQL_RES *mysqlRes = NULL;
	if(mysql_real_query(&m_connection, strColum.c_str(), strColum.length()) != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return vecTemp;
	}
	if((mysqlRes = mysql_store_result(&m_connection)) == NULL) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return vecTemp;
	}

	int rowLen = mysql_num_rows(mysqlRes);

	MYSQL_ROW row;
	unsigned int num_fields;
	unsigned int i;
	num_fields = mysql_num_fields(mysqlRes);
	string strTem;
	while ((row = mysql_fetch_row(mysqlRes)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(mysqlRes);
		for(i = 0; i < num_fields; i++)
		{
			strTem+=(row[i] ? row[i] : "NULL");
			strTem+=",";
		}

		strTem = strTem.substr(0,strTem.rfind(","));
		vecTemp.push_back(strTem);
		strTem.erase();
	}
	
	return vecTemp;
}

CMysqlQueue::return_val CMysqlQueue::getQueryKeysData(const string strColum,vector<string> &vecColum){
	if(checkConnectAlive() == FALSE)
		return mysql_dbDisconnection;
	MYSQL_RES *mysqlRes = NULL;
	if(mysql_real_query(&m_connection, strColum.c_str(), strColum.length()) != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	if((mysqlRes = mysql_store_result(&m_connection)) == NULL) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}

	int rowLen = mysql_num_rows(mysqlRes);
	MYSQL_ROW row;
	unsigned int num_fields;
	unsigned int i;
	num_fields = mysql_num_fields(mysqlRes);
	string strTem;
	while ((row = mysql_fetch_row(mysqlRes)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(mysqlRes);
		for(i = 0; i < num_fields; i++)
		{
			strTem+=(row[i] ? row[i] : "NULL");
			strTem+=",";
		}

		strTem = strTem.substr(0,strTem.rfind(","));
		vecColum.push_back(strTem);
		strTem.erase();
	}

	return mysql_success;
}


CMysqlQueue::return_val CMysqlQueue::getQueryData(const string strQuery,const string strColum,const string tableName){
	if(checkConnectAlive() == FALSE)
		return mysql_dbDisconnection;
	MYSQL_RES *mysqlRes = NULL;

	//得到当前查询时间
	char strTime[64]; 
	time_t t2 = time(0); 
	strftime( strTime, sizeof(strTime), "%Y-%m-%d %X",localtime(&t2) );	//得到当前时间

	if(mysql_real_query(&m_connection, strQuery.c_str(), strQuery.length()) != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	if((mysqlRes = mysql_store_result(&m_connection)) == NULL) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	
	int rowLen = mysql_num_rows(mysqlRes);

	MYSQL_ROW row;
	unsigned int num_fields;
	unsigned int i;
	num_fields = mysql_num_fields(mysqlRes);
	P_GENERATE_DATA pushData = new GENERATE_DATA;
	pushData->tableName_ = tableName;
	pushData->columnKey_ =getQueryData(strColum);
	while ((row = mysql_fetch_row(mysqlRes)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(mysqlRes);
		vector<string> vecTem;
		for(i = 0; i < num_fields; i++)
		{
			string rTem = (row[i] ? row[i] : "NULL");
			if (rTem.size()==0)
			{
				rTem = "NULL";
			}
			vecTem.push_back(rTem);
		}
		if (vecTem.size()>0)
		{
			pushData->columnVal_ = vecTem;
		}
		if (pushData->columnVal_.size()>0)
		{
			ProductDataGenerate(pushData);
			//free(pushData);
		}
	}

	//更改时间戳
	if (pushData->columnVal_.size()>0)
	{
		WriteTimeUpdateXML(tableName,strTime);
	}

	return mysql_success;
}

CMysqlQueue::return_val CMysqlQueue::getQueryData(const string strQuery,const string tableName,vector<vector<string>> &vecValues){
	if(checkConnectAlive() == FALSE)
		return mysql_dbDisconnection;
	MYSQL_RES *mysqlRes = NULL;
	if(mysql_real_query(&m_connection, strQuery.c_str(), strQuery.length()) != 0) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}
	if((mysqlRes = mysql_store_result(&m_connection)) == NULL) {
		setError();
		THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
		return mysql_dbExecFailed;
	}

	int rowLen = mysql_num_rows(mysqlRes);
	MYSQL_ROW row;
	unsigned int num_fields;
	unsigned int i;
	num_fields = mysql_num_fields(mysqlRes);
	while ((row = mysql_fetch_row(mysqlRes)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(mysqlRes);
		vector<string> vecTem;
		vecTem.clear();
		for(i = 0; i < num_fields; i++)
		{
			string rTem = (row[i] ? row[i] : "NULL");
			if (rTem.size()==0)
			{
				rTem = "NULL";
			}
			vecTem.push_back(rTem);
		}

		vecValues.push_back(vecTem);
		vecTem.clear();
	}

	return mysql_success;
}

BOOL CMysqlQueue::PutMessageQueue(const string msgHeadKey, const string tableName,const vector<string> vecKeys,vector<vector<string>> vecValues)
{
	vector<P_HEAD_GENERATE_DATA> vecQueueHead;
	vecQueueHead.clear();

	if(vecValues.size()<=0 || vecKeys.size()<=0)
	{
		return FALSE;
	}

	//生产数据
	long execData = 2;
	long lNum = vecValues.size()/execData;
	int iRemainder = vecValues.size()%execData;
	if (vecValues.size()<=execData)
	{
		vector<P_GENERATE_DATA> vecQueuePro;
		vecQueuePro.clear();
		P_HEAD_GENERATE_DATA pHeadData = new HEAD_GENERATE_DATA;
		for (long nIndex=0;nIndex<vecValues.size();nIndex++)
		{
			if (vecValues[nIndex].size()>0)
			{
				P_GENERATE_DATA pushData = new GENERATE_DATA;
				pushData->tableName_ =tableName;
				pushData->columnKey_ =vecKeys;
				pushData->columnVal_ = vecValues[nIndex];
				vecQueuePro.push_back(pushData);
				//free(pushData);
			}
		}
		pHeadData->headID_=msgHeadKey;
		pHeadData->headNumber_=1;
		pHeadData->headOrder_=1;
		pHeadData->headBody=vecQueuePro;
		ProductDataGenerate(pHeadData);
	}
	else
	{
		if(iRemainder != 0)
		{
			lNum+=1;
		}
		for (long jIndex  =0;jIndex<lNum;jIndex++)
		{
			vector<P_GENERATE_DATA> vecQueueBig;
			vecQueueBig.clear();
			if ((jIndex == lNum -1) && (iRemainder !=0))
			{
				if (iRemainder != 0)
				{
					for (long nIndex=0;nIndex<execData-iRemainder;nIndex++)
					{
						if (vecValues[(execData-iRemainder)*jIndex+nIndex].size()>0)
						{
							P_GENERATE_DATA pushData = new GENERATE_DATA;
							pushData->tableName_ = tableName;
							pushData->columnKey_ = vecKeys;
							pushData->columnVal_ = vecValues[(execData-iRemainder)*jIndex+nIndex];
							vecQueueBig.push_back(pushData);
						}
					}
				}
			}
			else
			{
				for (long nIndex=0;nIndex<execData;nIndex++)
				{
					if (vecValues[(execData)*jIndex+nIndex].size()>0)
					{
						P_GENERATE_DATA pushData = new GENERATE_DATA;
						pushData->tableName_ = tableName;
						pushData->columnKey_ = vecKeys;
						pushData->columnVal_ = vecValues[execData*jIndex+nIndex];
						vecQueueBig.push_back(pushData);
					}
				}
			}

			P_HEAD_GENERATE_DATA pHeadData = new HEAD_GENERATE_DATA;
			pHeadData->headID_=msgHeadKey;
			pHeadData->headNumber_=lNum;
			pHeadData->headOrder_=jIndex+1;
			pHeadData->headBody=vecQueueBig;
			ProductDataGenerate(pHeadData);
		}
	}
	
	return TRUE;
}

BOOL CMysqlQueue::PutOneMessageQueue(const string msgHeadKey,const long totalNum,const long theFirstFew, const string tableName,const vector<string> vecKeys,vector<vector<string>> vecValues)
{
	if(vecValues.size()<=0 || vecKeys.size()<=0)
	{
		return FALSE;
	}

	//生产数据
	long execData = 2;
	long lNum = vecValues.size()/execData;
	int iRemainder = vecValues.size()%execData;
	if (vecValues.size()<=execData)
	{
		vector<P_GENERATE_DATA> vecQueuePro;
		vecQueuePro.clear();
		P_HEAD_GENERATE_DATA pHeadData = new HEAD_GENERATE_DATA;
		for (long nIndex=0;nIndex<vecValues.size();nIndex++)
		{
			if (vecValues[nIndex].size()>0)
			{
				P_GENERATE_DATA pushData = new GENERATE_DATA;
				pushData->tableName_ =tableName;
				pushData->columnKey_ =vecKeys;
				pushData->columnVal_ = vecValues[nIndex];
				vecQueuePro.push_back(pushData);
				//free(pushData);
			}
		}
		pHeadData->headID_=msgHeadKey;
		pHeadData->headNumber_=totalNum;
		pHeadData->headOrder_=theFirstFew;
		pHeadData->headBody=vecQueuePro;
		vecQueuePro.clear();
		ProductDataGenerate(pHeadData);
		//free(pHeadData);
	}
	else
	{
		if(iRemainder != 0)
		{
			lNum+=1;
		}
		for (long jIndex  =0;jIndex<lNum;jIndex++)
		{
			vector<P_GENERATE_DATA> vecQueueBig;
			vecQueueBig.clear();
			if ((jIndex == lNum -1) && (iRemainder !=0))
			{
				if (iRemainder != 0)
				{
					for (long nIndex=0;nIndex<execData-iRemainder;nIndex++)
					{
						if (vecValues[(execData-iRemainder)*jIndex+nIndex].size()>0)
						{
							P_GENERATE_DATA pushData = new GENERATE_DATA;
							pushData->tableName_ = tableName;
							pushData->columnKey_ = vecKeys;
							pushData->columnVal_ = vecValues[(execData-iRemainder)*jIndex+nIndex];
							vecQueueBig.push_back(pushData);
							//free(pushData);
						}
					}
				}
			}
			else
			{
				for (long nIndex=0;nIndex<execData;nIndex++)
				{
					if (vecValues[(execData)*jIndex+nIndex].size()>0)
					{
						P_GENERATE_DATA pushData = new GENERATE_DATA;
						pushData->tableName_ = tableName;
						pushData->columnKey_ = vecKeys;
						pushData->columnVal_ = vecValues[execData*jIndex+nIndex];
						vecQueueBig.push_back(pushData);
						//free(pushData);
					}
				}
			}

			P_HEAD_GENERATE_DATA pHeadData = new HEAD_GENERATE_DATA;
			pHeadData->headID_=msgHeadKey;
			pHeadData->headNumber_=totalNum;
			pHeadData->headOrder_=theFirstFew+jIndex+1;
			pHeadData->headBody=vecQueueBig;
			vecQueueBig.clear();
			ProductDataGenerate(pHeadData);
			//free(pHeadData);
			/*delete []pHeadData;*/
		}
	}

	return TRUE;
}

BOOL CMysqlQueue::PutMessageQueue(const string msgHeadKey, const vector<string> tableName,const vector<vector<string>> vecKeys,const vector<vector<vector<string>>> vecValues)
{
	if(vecValues.size()<=0 || vecKeys.size()<=0)
	{
		return FALSE;
	}
	
	long execData = 2;
	long SendNumber=0;//查询多个表的总的记录数
	for (int iNum=0;iNum<vecValues.size();iNum++)
	{
		//long lNum = SendNumber/execData;
		int iRemainder = vecValues[iNum].size()%execData;
		if(iRemainder!=0)
		{
			SendNumber+=1;
		}
		SendNumber+=vecValues[iNum].size()/execData;
	}
	
	//生产数据
	if (SendNumber<=execData)
	{
		vector<P_GENERATE_DATA> vecQueuePro;
		vecQueuePro.clear();
		P_HEAD_GENERATE_DATA pHeadData = new HEAD_GENERATE_DATA;
		for (long nIndex=0;nIndex<vecValues[0].size();nIndex++)
		{
			if (vecValues[nIndex].size()>0)
			{
				P_GENERATE_DATA pushData = new GENERATE_DATA;
				pushData->tableName_ =tableName[0];
				pushData->columnKey_ =vecKeys[0];
				pushData->columnVal_ = vecValues[0][nIndex];
				vecQueuePro.push_back(pushData);
				//free(pushData);
			}
		}
		pHeadData->headID_=msgHeadKey;
		pHeadData->headNumber_=1;
		pHeadData->headOrder_=1;
		pHeadData->headBody=vecQueuePro;
		vecQueuePro.clear();
		ProductDataGenerate(pHeadData);
		//free(pHeadData);
	}
	else
	{
		//发送集合每一个表的
		long lSendNum=0;
		for (int iSendTable=0;iSendTable<vecValues.size();iSendTable++)
		{
			vector<vector<string >> vvv=vecValues[iSendTable];
			vector<string> kkk = vecKeys[iSendTable];
			string nnn = tableName[iSendTable];
			long ln = lSendNum;
			
			PutOneMessageQueue(msgHeadKey,SendNumber,lSendNum,tableName[iSendTable],vecKeys[iSendTable],vecValues[iSendTable]);
			//PutOneMessageQueue(msgHeadKey,SendNumber,lSendNum,tableName[iSendTable],vecKeys[iSendTable],vecValues[iSendTable]);
			int jRemainder = vecValues[iSendTable].size()%execData;
			if(jRemainder!=0)
			{
				lSendNum+=1;
			}
			lSendNum+=vecValues[iSendTable].size()/execData;
		}
		
	}

	return TRUE;
}


BOOL CMysqlQueue::PutOneMessageQueue(const string msgHeadKey, const vector<string> tableName,const vector<vector<string>> vecKeys,const vector<vector<vector<string>>> vecValues)
{
	if(vecValues.size()<=0 || vecKeys.size()<=0)
	{
		return FALSE;
	}

	long SendNumber=0;//查询多个表的总的记录数
	for (int iNum=0;iNum<vecValues.size();iNum++)
	{
		SendNumber+=vecValues[iNum].size();
	}

	long SendPro=0;
	for (int iTabl=0;iTabl<vecValues.size();iTabl++)
	{
		for (long nIndex=0;nIndex<vecValues[iTabl].size();nIndex++)
		{
			vector<P_GENERATE_DATA> vecQueuePro;
			vecQueuePro.clear();
			P_HEAD_GENERATE_DATA pHeadData = new HEAD_GENERATE_DATA;
			P_GENERATE_DATA pushData = new GENERATE_DATA;
			pushData->tableName_ =tableName[iTabl];
			pushData->columnKey_ =vecKeys[iTabl];
			pushData->columnVal_ = vecValues[iTabl][nIndex];
			vecQueuePro.push_back(pushData);
			pHeadData->headID_=msgHeadKey;
			pHeadData->headNumber_=SendNumber;
			pHeadData->headOrder_=++SendPro;
			pHeadData->headBody=vecQueuePro;
			ProductDataGenerate(pHeadData);
		}
	}

	return TRUE;
}


CMysqlQueue::return_val CMysqlQueue::execQuerySql(const string msgHeadKey,const string strQuery,const string strColum,const string tableName) {
	MUTEX_LOCK(MysqlQueue);
	vector<string> vecKeys;
	vecKeys.clear();
	vector<vector<string>> vecValues;
	vecValues.clear();
	if(getQueryKeysData(strColum,vecKeys) != mysql_success) {//表中的字段名
		goto error;
	}
	if(getQueryData(strQuery,tableName,vecValues) != mysql_success) { //表中的所有数据
		goto error;
	}
	mysql_commit(&m_connection);

	PutMessageQueue(msgHeadKey,tableName,vecKeys,vecValues);
	return mysql_success;
error:
	setError();
	THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
	mysql_rollback(&m_connection);
	return mysql_dbMuilFailed;
}

CMysqlQueue::return_val CMysqlQueue::execQuerySql(const string strQuery,const string strColum,const string tableName,vector<string> &queueTable,vector<vector<string>> &vecKeys,vector<vector<vector<string>>> &vecValues) {
	MUTEX_LOCK(MysqlQueue);
	vector<string> vecOneKey;
	vector<vector<string>> vecOneValue;
	vecOneKey.clear();
	vecOneValue.clear();
	if(getQueryKeysData(strColum,vecOneKey) != mysql_success) {//表中的字段名
		goto error;
	}

	if(getQueryData(strQuery,tableName,vecOneValue) != mysql_success) { //表中的所有数据
		goto error;
	}
	if(vecOneValue.size()>0)
	{
		queueTable.push_back(tableName);
		vecKeys.push_back(vecOneKey);
		vecValues.push_back(vecOneValue);

		vecOneKey.clear();
		vecOneValue.clear();
	}
	mysql_commit(&m_connection);
	return mysql_success;
error:
	return mysql_dbMuilFailed;
}


VOID CMysqlQueue::ProductData(P_GENERATE_DATA getDate) {
	ENTRY_MUTEX(CMysqlQueue);
	if(m_queue_data.size() >= ROWS_DATA_MAX_SIZE) {
		CONDITION_SLEEP(CMysqlQueue, full);
	}
	else {
		m_queue_data.push_back(getDate);
	}
	CONDITION_WAKEUP(CMysqlQueue, empty);
	LEAVE_MUTEX(CMysqlQueue);
}

P_GENERATE_DATA CMysqlQueue::ConsumerData() {
	ENTRY_MUTEX(CMysqlQueue);
	string rvStr = "";
	P_GENERATE_DATA mapData;
	if(m_queue_data.size() == 0) {
		CONDITION_SLEEP(CMysqlQueue, empty);
	}
	if(m_queue_data.size() > 0) {
		mapData = m_queue_data.front();
		m_queue_data.pop_front();
	}

	CONDITION_WAKEUP(CMysqlQueue, full);
	LEAVE_MUTEX(CMysqlQueue);
	return mapData;
}

CMysqlQueue::return_val CMysqlQueue::execMutilSql(const vector<string> sqlSelec,const vector<string> strColum,const vector<string> tableName) {
	MUTEX_LOCK(MysqlQueue);
	int sqlSize = sqlSelec.size();
	unsigned long long rowNum = 0;
	
	vector<vector<string>> vecKeysTable;
	vector<vector<vector<string>>> vecValuesTable;
	vector<vector<string>> vecProKeysTable;
	vector<vector<vector<string>>> vecProValuesTable;
	static	vector<vector<string>> preVecKeysTable;
	static  vector<vector<vector<string>>> preVecValuesTable;
	vecKeysTable.clear();
	vecValuesTable.clear();
	vector<string> queueFindTableName;
	vector<string> ProQeueFindTableName;
	vector<string> PreQeueFindTableName;

	//得到当前查询时间	,产生head唯一标示
	char strTime[64]; 
	time_t t2 = time(0); 
	strftime( strTime, sizeof(strTime), "%Y-%m-%d %X",localtime(&t2) );
	string sTimeTag = strTime;	
	MD5 iMD5;	
	iMD5.GenerateMD5((unsigned char *)sTimeTag.c_str(), sTimeTag.size());
	string headID = iMD5.ToString();
	for(int i = 0; i < 4; i++) {//B集合
		if(execQuerySql(sqlSelec[i],strColum[i],tableName[i],queueFindTableName,vecKeysTable,vecValuesTable) != mysql_success) {
			goto error;
		}
	}

	//集合b'
	for(int ij = 4; ij < sqlSize; ij++) {//B'集合
		if(execQuerySql(sqlSelec[ij],strColum[ij],tableName[ij],ProQeueFindTableName,vecProKeysTable,vecProValuesTable) != mysql_success) {
			goto error;
		}
	}
	if(PreQeueFindTableName.size()>0)
	{
		//放进队列之前先比较b‘和a;vecProKeysTable重复的去掉
		for (int IProKey=0;IProKey<ProQeueFindTableName.size();IProKey++)
		{
			int LPre;
			for (LPre=0;LPre<PreQeueFindTableName.size();LPre++)
			{
				if (ProQeueFindTableName[IProKey] == PreQeueFindTableName[LPre])
				{
					break;
				}
			}
			if (LPre==PreQeueFindTableName.size())
			{
				//如果在上次查询的时候没有这个表的数据直接添加对应的表中信息到发送的MSG----MQ
				int iRet;
				for (iRet=0;iRet<queueFindTableName.size();iRet++)
				{
					if (ProQeueFindTableName[IProKey] == queueFindTableName[iRet])
					{
						break;
					}
				}
				if(iRet == queueFindTableName.size())
				{//上次查询没有表
					queueFindTableName.push_back(ProQeueFindTableName[IProKey]);
					vecKeysTable.push_back(vecProKeysTable[IProKey]);
					vecValuesTable.push_back(vecProValuesTable[IProKey]);
				}
				else
				{//当前查询有对应表，每一条数据在添加在对应表
					for (long iValue=0;iValue<vecProValuesTable[IProKey].size();iValue++)
					{
						vecValuesTable[iRet].push_back(vecProValuesTable[IProKey][iValue]);
					}
				}
			}
			else
			{//有对应表，还要和上次数据进行比较,每一条数据在上次查询是否有
				for (long iValue=0;iValue<vecProValuesTable[IProKey].size();iValue++)
				{
					long lValue;
					for (lValue=0;lValue<preVecValuesTable[LPre].size();lValue++)
					{
						if (vecProValuesTable[IProKey][iValue] == preVecValuesTable[LPre][lValue])
						{
							break;
						}
					}
					if (lValue==preVecValuesTable[LPre].size())
					{
						int nRet;
						for (nRet=0;nRet<queueFindTableName.size();nRet++)
						{
							if (ProQeueFindTableName[IProKey] == queueFindTableName[nRet])
							{
								break;
							}
						}
						if(nRet == queueFindTableName.size())
						{//当前查询没有对改表没有增量数据
							queueFindTableName.push_back(ProQeueFindTableName[IProKey]);
							vecKeysTable.push_back(vecProKeysTable[IProKey]);
							vecValuesTable.push_back(vecProValuesTable[IProKey]);
						}
						else
						{//当前查询有对应表，且有增量数据
							for (long iValue=0;iValue<vecProValuesTable[IProKey].size();iValue++)
							{
								vecValuesTable[nRet].push_back(vecProValuesTable[IProKey][iValue]);
							}
						}
					}
				}
			}
		}
	}
	
	preVecKeysTable = vecKeysTable;
	preVecValuesTable = vecValuesTable;
	PutOneMessageQueue(headID,queueFindTableName,vecKeysTable,vecValuesTable);//没有问题的

	vecKeysTable.clear();
	vecValuesTable.clear();

	mysql_commit(&m_connection);
	return mysql_success;
error:
	setError();
	THROW_EXCEPTION(ExceptionDatabase, m_mysql_error_format);
	mysql_rollback(&m_connection);
	return mysql_dbMuilFailed;
}

unsigned long CMysqlQueue::getThreadId() {
	return m_threadId;
}

void CMysqlQueue::updateThreadId(){
	m_threadId = mysql_thread_id(&m_connection);
}

int CMysqlQueue::pingServer(){
	MUTEX_LOCK(MysqlQueue);
	int iRv = mysql_ping(&m_connection);
	return iRv;
}