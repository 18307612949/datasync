#include "StdAfx.h"
#include "PreBinlog.h"
#include <algorithm>
#include <tinyxml/tinyxml.h>
#include <ClientSocketServer.h>
#include <LogData.h>

CPreBinlog* CPreBinlog::m_instance = NULL;
CPreBinlog::CPreBinlog(void) {
	char bufName[MAX_PATH]={0};
	::GetPrivateProfileStringA(STOREHOUSE_CONFIG_KEY, "Name", "",bufName,MAX_PATH,CONFIG_PATH);
	m_storehouse_name =bufName;

	::GetPrivateProfileStringA(STOREHOUSE_CONFIG_KEY, "Code", "",bufName,MAX_PATH,CONFIG_PATH);
	m_storehouse_id = bufName;
}


CPreBinlog::~CPreBinlog(void) {
}

BOOL CPreBinlog::InitConfig() {
	TiXmlDocument doc;
	if(doc.LoadFile(TABLE_XML_PATH) == false) {
		return FALSE;
	}
	TiXmlElement* rootElement = doc.RootElement();
	if(rootElement == NULL)
		return FALSE;
	vector<string>().swap(m_careTableName);
	TiXmlElement* tableElement;
	for(tableElement = rootElement->FirstChildElement(); tableElement; tableElement = tableElement->NextSiblingElement()) {
		m_careTableName.push_back(tableElement->Attribute("name"));
	}
	if(m_careTableName.size() == 0)
		return FALSE;
	return TRUE;
}

BOOL CPreBinlog::FindTableName(const string tableName) {
	return (find(m_careTableName.begin(), m_careTableName.end(), tableName) == m_careTableName.end())?FALSE:TRUE;
}

DWORD CPreBinlog::ProcessBinlogThread(LPVOID lparam) {
	CPreBinlog *pInstance = (CPreBinlog*)lparam;
	while(true) {
		P_HEAD_GENERATE_DATA pData = NULL;
		pData = VectorHeadConsumerDataGenerate();
		if(pData == NULL) {
			continue;
		}

		string sendMSG =  pInstance->BuildSendData(pData);
		if(ClientSocketServer::getInstance()->SendMessage(MSG_TYPE_BINLOG, sendMSG))
			CLogData::getInstance()->WriteLog("RawData", sendMSG);
		else
			CLogData::getInstance()->WriteLog("FailRawData",  sendMSG);
		delete pData;
		
	}
	return 0;
}

string CPreBinlog::BuildSendData(P_HEAD_GENERATE_DATA pData) 
{
	if (pData == NULL )
	{
		return "";
	}

	// «tables
	Json::Value root;
	Json::Value tableArr;
	for (long lItIndex=0;lItIndex<pData->headBody.size();lItIndex++)
	{
		P_GENERATE_DATA pJsonDataChild=pData->headBody[lItIndex];
		//±Ì√¸
		Json::Value tables;
		tables["name"] = Json::Value(pJsonDataChild->tableName_);
		//column
		Json::Value json_column;
		for(int nIndex = 0;nIndex<pJsonDataChild->columnKey_.size();nIndex++)
		{
			tables["column"].append(pJsonDataChild->columnKey_[nIndex].c_str());             
		}
		json_column.clear();
		//values
		Json::Value json_values;
		for(int iRet=0;iRet<pJsonDataChild->columnVal_.size();iRet++)
		{
			tables["values"].append(pJsonDataChild->columnVal_[iRet].c_str());
		}
		json_values.clear();
		root["tables"].append(tables);
		tables.clear();
	}
	Json::FastWriter fast_writer;
	string strJson;
	char *pBuf = (char *)malloc(64);
	strJson+=pData->headID_;
	strJson+=",";
	strJson+=ltoa(pData->headNumber_,pBuf,10);
	strJson+=",";
	strJson+=ltoa(pData->headOrder_,pBuf,10);
	strJson+=",";
	strJson+=m_storehouse_id;//≤÷ø‚ID
	//strJson+=",";
	strJson+=fast_writer.write(root);

	delete []pBuf;

	return strJson.substr(0,strJson.length()-1);
}

BOOL CPreBinlog::StartProcessBinlog() {
	if(InitConfig() == FALSE) {
		return FALSE;
	}
	m_processThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessBinlogThread, this, 0, (DWORD*)&m_processThreadExitCode);
	return TRUE;
}

CPreBinlog* CPreBinlog::getInstance(){
	if(m_instance == NULL) {
		MUTEX_LOCK(CPreBinlog);
		if(m_instance == NULL){
			m_instance = new CPreBinlog();
		}
	}
	return m_instance;
}