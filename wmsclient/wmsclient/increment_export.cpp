#include "StdAfx.h"
#include "increment_export.h"

#include <tinyxml/tinyxml.h>
#include <LogData.h>
#include <LogError.h>
#include <lock.h>
#include "increment_export.h"
#include "MyslqQueue.h"
#include <windows.h>
#include <time.h>
#define TABLE_XML_PATH "config/tableinfo.xml"
#define ROWS_DATA_MAX_SIZE 1000

vector<string> glb_table_name;
vector<string> glb_timestamp;
DWORD glb_readDBThreadId;
int glb_period_time; //时间间隔

bool isRunning = true;

deque<P_GENERATE_DATA>	glb_queue_data;
deque<vector<P_HEAD_GENERATE_DATA>> glb_vec_queue_data;
deque<P_HEAD_GENERATE_DATA>	glb_head_queue_data;
//string glb_dbconnect;

string InitDateBase()
{
	////读取数据库配置信息
	string strDBInfo = "config\\DataBaseInfo.txt";
	char DBInfo[200] ={0};
	string strInfo;
	FILE *fp = fopen(strDBInfo.c_str(),"r");
	if (fp)
	{
		while(fgets(DBInfo,sizeof(DBInfo),fp))
		{
		} 

		fclose(fp);
		fp = NULL;	
	}

	strInfo = DBInfo;

	return strInfo;
}

bool InitConfig() {
	glb_table_name.clear();
	glb_timestamp.clear();

	TiXmlDocument doc;
	if(doc.LoadFile(TABLE_XML_PATH) == false) {
		return false;
	}
	TiXmlElement* rootElement = doc.RootElement();
	if(rootElement == NULL)
		return false;
	TiXmlElement* tableElement;
	//读取数据时间间隔
	string time_break = rootElement->Attribute("period_time");
	glb_period_time = atoi(time_break.c_str());
	for(tableElement = rootElement->FirstChildElement(); tableElement; tableElement = tableElement->NextSiblingElement()) {
		string name = tableElement->Attribute("name");
		string timetasp = tableElement->Attribute("hm_update_timestamp");
		glb_table_name.push_back(name);
		glb_timestamp.push_back(timetasp);
	}
	if(glb_table_name.size() == 0 || glb_timestamp.size() == 0)
		return false;

	return true;
}

bool WriteTimeUpdateXML(const string tableName,const string keyValues)
{

	MUTEX_LOCK(XMLWrite);
	TiXmlDocument doc;
	if(doc.LoadFile(TABLE_XML_PATH) == false) {
		return false;
	}
	TiXmlElement* rootElement = doc.RootElement();
	if(rootElement == NULL)
		return false;
	TiXmlElement* tableElement;
	for(tableElement = rootElement->FirstChildElement(); tableElement; tableElement = tableElement->NextSiblingElement()) {
		string name = tableElement->Attribute("name");
		if (name==tableName)
		{
			tableElement->SetAttribute("hm_update_timestamp",keyValues.c_str());
		}
	}

	doc.SaveFile(TABLE_XML_PATH);

	return true;
}

string GetStrFromTime(time_t iTimeStamp/*, char *pszTime*/)  
{  
	char pszTime[24]={0};
	tm *pTmp = localtime(&iTimeStamp);  
	if (pTmp == NULL)  
	{  
		return FALSE;  
	}  
	sprintf(pszTime, "%d-%d-%d %d:%d:%d", pTmp->tm_year + 1900, pTmp->tm_mon + 1, pTmp->tm_mday, pTmp->tm_hour, pTmp->tm_min, pTmp->tm_sec);  
	string RetTime = pszTime;
	return RetTime;  
}

unsigned int GetTimeStampByStr( const char* pDate/*, int32 iNameSize*/ )   
{  
   const char* pStart = pDate;  
     
   char szYear[5], szMonth[3], szDay[3], szHour[3], szMin[3], szSec[3];  
     
   szYear[0]   = *pDate++;  
   szYear[1]   = *pDate++;  
   szYear[2]   = *pDate++;  
   szYear[3]   = *pDate++;  
   szYear[4]   = 0x0;  
     
   ++pDate;  
     
   szMonth[0]  = *pDate++;  
   szMonth[1]  = *pDate++;  
   szMonth[2]  = 0x0;  
     
   ++pDate;  
     
   szDay[0]    = *pDate++;   
   szDay[1]    = *pDate++;  
   szDay[2]    = 0x0;  
     
   ++pDate;  
     
   szHour[0]   = *pDate++;  
   szHour[1]   = *pDate++;  
   szHour[2]   = 0x0;  
     
   ++pDate;  
     
   szMin[0]    = *pDate++;  
   szMin[1]    = *pDate++;  
   szMin[2]    = 0x0;  
     
   ++pDate;  
     
   szSec[0]    = *pDate++;  
   szSec[1]    = *pDate++;  
   szSec[2]    = 0x0;  
     
   tm tmObj;  
     
   tmObj.tm_year = atoi(szYear)-1900;  
   tmObj.tm_mon  = atoi(szMonth)-1;  
   tmObj.tm_mday = atoi(szDay);  
   tmObj.tm_hour = atoi(szHour);  
   tmObj.tm_min  = atoi(szMin);  
   tmObj.tm_sec  = atoi(szSec);  
   tmObj.tm_isdst= -1;  
     
   return mktime(&tmObj);
   //return tmObj;
}  

string GetWriteXmlTime(const string sTime)
{
	if (sTime.size() == 19)
	{
		return sTime;
	}

	//2016-05-03 03:05:08
	string  syear,smon,sday,shour,smin,ssec;
	string temp;
	temp=sTime.substr(sTime.find(':')+1,sTime.size()-sTime.find(':')-1);
	syear =sTime.substr(0,4);
	smon = sTime.substr(5,sTime.find_last_of('-')-5);
	if (smon.size() != 2)
	{
		smon.insert(0,"0");
	}
	sday = sTime.substr(sTime.find_last_of('-')+1,sTime.find(" ")-sTime.find_last_of('-')-1);
	if (sday.size() != 2)
	{
		sday.insert(0,"0");
	}
	shour= sTime.substr(sTime.find(" ")+1,sTime.find(':')-sTime.find(" ")-1);
	if (shour.size() != 2)
	{
		shour.insert(0,"0");
	}
	smin=temp.substr(0,temp .find(':'));
	if (smin.size() != 2)
	{
		smin.insert(0,"0");
	}
	ssec = sTime.substr(sTime.find_last_of(':')+1,sTime.size()-sTime.find_last_of(':')-1);
	if (ssec.size() != 2)
	{
		ssec.insert(0,"0");
	}

	string reTemp;
	reTemp+=syear;
	reTemp+="-";
	reTemp+=smon;
	reTemp+="-";
	reTemp+=sday;
	reTemp+=" ";
	reTemp+=shour;
	reTemp+=":";
	reTemp+=smin;
	reTemp+=":";
	reTemp+=ssec;

	return reTemp;
}

void GetSelectSql(vector<string> &selectStampSql,vector<string> &selectCurSql)
{
	selectStampSql.clear();
	selectCurSql.clear();
	for (int nIndex=0;nIndex<glb_table_name.size();nIndex++)
	{
		string strSelectSQL;
		strSelectSQL="select COLUMN_NAME from information_schema.COLUMNS where table_name='";
		strSelectSQL+=glb_table_name[nIndex];
		strSelectSQL+="'";

		//得到上次取数据的时间戳
		int nTime =GetTimeStampByStr(glb_timestamp[0].c_str());
		nTime-=glb_period_time;												//应该是这个glb_period_time，为了方便测试设置成2个小时间隔;
		string preQueryTime = GetStrFromTime(nTime);

		string strSelectValue;
		strSelectValue="select * from ";
		strSelectValue+=glb_table_name[nIndex];
		strSelectValue+=" where hm_update_timestamp >'";
		strSelectValue+=preQueryTime;								//glb_timestamp[nIndex] -30秒
		strSelectValue+="' and hm_update_timestamp <='";
		strSelectValue+= glb_timestamp[nIndex];
		strSelectValue+="'";

		selectStampSql.push_back(strSelectSQL);
		selectCurSql.push_back(strSelectValue);
	}

	for (int nndex=0;nndex<glb_table_name.size();nndex++)
	{
		//得到上次取数据的时间戳
		int nTime =GetTimeStampByStr(glb_timestamp[0].c_str());
		nTime-=glb_period_time*2;												//应该是这个glb_period_time，为了方便测试设置成2个小时间隔;
		string preQueryTime = GetStrFromTime(nTime);

		nTime =GetTimeStampByStr(glb_timestamp[0].c_str());
		nTime-=glb_period_time*4;												//应该是这个2*glb_period_time，为了方便测试设置成2个小时间隔;
		string prePreQueryTime = GetStrFromTime(nTime);

		string strSelectValue;
		strSelectValue="select * from ";
		strSelectValue+=glb_table_name[nndex];
		strSelectValue+=" where hm_update_timestamp >'";
		strSelectValue+=prePreQueryTime;
		strSelectValue+="' and hm_update_timestamp <='";
		strSelectValue+= preQueryTime;
		strSelectValue+="'";

		selectCurSql.push_back(strSelectValue);
	}
}

//读取数据和标的字段；
void readData()
{
	if (glb_table_name.size() == 0 || glb_timestamp.size() == 0)
	{
		return;
	}

	MUTEX_LOCK(ReadDBIncrement);
	vector<string> strSelectValuesTemp;
	vector<string> strSelectKeyTemp;
	GetSelectSql(strSelectKeyTemp,strSelectValuesTemp);

	if(CMysqlQueue::mysql_success==CMysqlQueue::getInstance()->execMutilSql(strSelectValuesTemp,strSelectKeyTemp,glb_table_name))
	{
		long nTime;
		nTime =GetTimeStampByStr(glb_timestamp[0].c_str());
		nTime+=glb_period_time;									//应该是这个glb_period_time	//更改时间戳
		string strQueryTime = GetStrFromTime(nTime);
		strQueryTime=GetWriteXmlTime(strQueryTime);
		for (int i=0;i<glb_table_name.size();i++)
		{
			WriteTimeUpdateXML(glb_table_name[i],strQueryTime.c_str());
		}
	}

	strSelectValuesTemp.clear();
	strSelectKeyTemp.clear();
}

#include <BuildFinally.h>
void getTestdata()
{
	vector<string> strSelectKeyTemp;
	string strQuery="select ew_download_to_mq_log.message from ew_download_to_mq_log";
	vector<vector<string>> vecValues;
	if(CMysqlQueue::mysql_success==/*CMysqlQueue::getInstance()->execMutilSql(strSelectValuesTemp,strSelectKeyTemp,glb_table_name)*/
		CMysqlQueue::getInstance()->getQueryData(strQuery,"ew_download_to_mq_log",vecValues))
	{
		vector<vector<vector<string>>> vecSameValues;
	
		for (int nIndex=0;nIndex<vecValues.size();nIndex++)
		{
			vector<string> ll=vecValues[nIndex];
			int nRet=ll[0].find("customers");
			if (nRet>0)
			{
				continue;
			}
			else
			{//不是客资表
				CBuildFinally buildInstance(ll[0]);
				buildInstance.LoadJson();
				buildInstance.m_sql_insert;
				for (int inser=0;inser<buildInstance.m_sql_insert.size();inser++)
				{
					nRet=buildInstance.m_sql_insert[inser].find("wm_so_detail ");
					if (nRet>0)
					{
						string sql=buildInstance.m_sql_insert[inser];
						sql =sql.substr(sql.rfind("\"120\""),sql.size());
						sql =sql.substr(sql.find(',')+1,sql.size());
						sql =sql.substr(sql.find(',')+1,sql.size());
						sql =sql.substr(sql.find(',')+1,sql.size());
						sql =sql.substr(1,sql.find(',')-2);
						//
						strQuery="select ew_download_to_mq_log.message from ew_download_to_mq_log where ew_download_to_mq_log.message like '%wm_so_detail%";
						strQuery+=sql;
						strQuery+="%'";
						
						vector<vector<string>> Temp;
						CMysqlQueue::getInstance()->getQueryData(strQuery,"ew_download_to_mq_log",Temp);
						if (Temp.size()>=2)
						{
							vecSameValues.push_back(Temp);
						}
					}
				}
				buildInstance.m_sql_update;
			}

		}
	}
}

DWORD WINAPI ThreadProcServer(PVOID pParam) {

	while (isRunning)
	{
		//初始化表和时间戳
		InitConfig();
		
		//getTestdata();

		readData();

		if (glb_period_time !=0)
		{
			int iTime = glb_period_time*1000;
			Sleep(iTime);
		}
		else
		{
			Sleep(30000);
		}
	}
		
	return 0;
}

void CALLBACK TimerProc(HWND hwnd, UINT Msg, UINT idEvent, DWORD dwTime)
{
	//初始化表和时间戳
	InitConfig();

	readData();
}

HANDLE glb_readHandle=NULL;
void StartGenerateData()
{
	//启动线程
	isRunning = true;
	glb_readHandle= CreateThread(NULL, 0, ThreadProcServer, NULL, CREATE_SUSPENDED, &glb_readDBThreadId);
	if(glb_readHandle == NULL) {
		CLogError::getInstance()->WriteErrorLog(::GetLastError(), "create read db  thread failed");
		return ;
	}
	ResumeThread(glb_readHandle);
	CloseHandle(glb_readHandle);
}


 VOID ProductDataGenerate(vector<P_HEAD_GENERATE_DATA> getDate/*P_GENERATE_DATA getDate*/) {
	ENTRY_MUTEX(IncrementDataLock);
	if(glb_vec_queue_data.size() >= ROWS_DATA_MAX_SIZE) {
		CONDITION_SLEEP(IncrementDataLock, full);
	}
	else {
		glb_vec_queue_data.push_back(getDate);
	}
	CONDITION_WAKEUP(IncrementDataLock, empty);
	LEAVE_MUTEX(IncrementDataLock);
}

 VOID ProductDataGenerate(P_HEAD_GENERATE_DATA getDate/*P_GENERATE_DATA getDate*/) {
	 ENTRY_MUTEX(IncrementDataLock);
	 if(glb_head_queue_data.size() >= ROWS_DATA_MAX_SIZE) {
		 CONDITION_SLEEP(IncrementDataLock, full);
	 }
	 else {
		 glb_head_queue_data.push_back(getDate);
	 }
	 CONDITION_WAKEUP(IncrementDataLock, empty);
	 LEAVE_MUTEX(IncrementDataLock);
 }

 VOID ProductDataGenerate(P_GENERATE_DATA getDate) {
	 ENTRY_MUTEX(IncrementDataLock);
	 if(glb_queue_data.size() >= ROWS_DATA_MAX_SIZE) {
		 CONDITION_SLEEP(IncrementDataLock, full);
	 }
	 else {
		 glb_queue_data.push_back(getDate);
	 }
	 CONDITION_WAKEUP(IncrementDataLock, empty);
	 LEAVE_MUTEX(IncrementDataLock);
 }

P_GENERATE_DATA ConsumerDataGenerate() {
	ENTRY_MUTEX(IncrementDataLock);
	GENERATE_DATA* mapData;
	if(glb_queue_data.size() == 0) {
		CONDITION_SLEEP(IncrementDataLock, empty);
	}
	if(glb_queue_data.size() > 0) {
		mapData = glb_queue_data.front();
		glb_queue_data.pop_front();
	}

	CONDITION_WAKEUP(IncrementDataLock, full);
	LEAVE_MUTEX(IncrementDataLock);
	return mapData;
}

P_HEAD_GENERATE_DATA VectorHeadConsumerDataGenerate() {
	ENTRY_MUTEX(IncrementDataLock);
	P_HEAD_GENERATE_DATA mapData;
	if(glb_head_queue_data.size() == 0) {
		CONDITION_SLEEP(IncrementDataLock, empty);
	}
	if(glb_head_queue_data.size() > 0) {
		mapData = glb_head_queue_data.front();
		glb_head_queue_data.pop_front();
	}

	CONDITION_WAKEUP(IncrementDataLock, full);
	LEAVE_MUTEX(IncrementDataLock);
	return mapData;
}

vector<P_HEAD_GENERATE_DATA> VectorConsumerDataGenerate() {
	ENTRY_MUTEX(IncrementDataLock);
	vector<P_HEAD_GENERATE_DATA> mapData;
	if(glb_vec_queue_data.size() == 0) {
		CONDITION_SLEEP(IncrementDataLock, empty);
	}
	if(glb_vec_queue_data.size() > 0) {
		mapData = glb_vec_queue_data.front();
		glb_vec_queue_data.pop_front();
	}

	CONDITION_WAKEUP(IncrementDataLock, full);
	LEAVE_MUTEX(IncrementDataLock);
	return mapData;
}

void StopGenerateData()
{
	isRunning = false;
	//KillTimer(NULL,TIMER_CLIENT_READ_DB_DATA);
	::WaitForSingleObject(glb_readHandle,INFINITE);
}





