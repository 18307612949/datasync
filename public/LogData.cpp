#include "LogData.h"
#include "lock.h"
#ifdef WIN32
#else
#include <stdarg.h>
#endif
//在此处初始化
CLogData* CLogData::m_Instance = NULL;
//int LogDataLock;
CLogData::CLogData()
{
	DirIsExits();
}

CLogData::~CLogData(void)
{

}

CLogData* CLogData::getInstance()
{
	//很多线程的话，就爱会造成大量线程的阻塞,这样写降低了线程的阻塞的几率
	if(m_Instance == NULL)
	{
		MUTEX_LOCK(LogDataLock);
		if (m_Instance == NULL)
		{
			m_Instance = new CLogData();
		}
	}

	return m_Instance;
}

void CLogData::DirIsExits()
{
	char strPath[253]="./log";

	int iRet = access(strPath,0);
	if (iRet == -1)/*目录不存在*/
	{

#ifdef WIN32
		TCHAR exepath[MAX_PATH]={0};
		GetModuleFileName(NULL,exepath,MAX_PATH);
		char strCur[2048]={0};
		TcharToChar(exepath,strCur);
		string s = strCur;
		string sub =  s.substr(0,s.rfind('\\'));

		char strDir[253]={0};
		sprintf(strDir,"%s\\log\\logData",sub.c_str());
		TCHAR tcreate[MAX_PATH]={0};
		CharToTchar(strDir,tcreate);
		SHCreateDirectory(0,tcreate);
#else
		char strPath[253]="mkdir log";
		char strChild[64]="mkdir log//logData";
		system(strPath);
		system(strChild);
#endif
	}
	else/*目录存在*/
	{
		char strPathChild[253]="log//logData";
		iRet = access(strPathChild,0);
		if (iRet == -1)/*目录不存在*/
		{

#ifdef WIN32
			TCHAR exepath[MAX_PATH]={0};
			GetModuleFileName(NULL,exepath,MAX_PATH);
			char strCur[2048]={0};
			TcharToChar(exepath,strCur);
			string s = strCur;
			string sub =  s.substr(0,s.rfind('\\'));

			char strDir[253]={0};
			sprintf(strDir,"%s\\log\\logData",sub.c_str());
			TCHAR tcreate[MAX_PATH]={0};
			CharToTchar(strDir,tcreate);
			SHCreateDirectory(0,tcreate);
#else
			char strChild[64]="mkdir log//logData";
			system(strChild);
#endif
		}
	}
}

int CLogData::WriteLog(const string &msg,const int operateCode)
{
	DirIsExits();//创建目录

	time_t t = time(0); 
	char strTime[64]; 
	strftime( strTime, sizeof(strTime), "%Y-%m-%d",localtime(&t) );	//年月日
	char logPath[MAX_PATH_SIZE];
	memset(logPath,0,MAX_PATH_SIZE);
	sprintf(logPath,".//log//logData//%s-logData.log",strTime);

	//在这里设置写入锁，防止同时写入文件
	MUTEX_LOCK(LogDataLock);
	int iRet = access(logPath,0);
	if (iRet == -1)
	{
		FILE *pFile;
		pFile = fopen(logPath, "a");
		if (pFile == NULL)
		{
			perror("文件打开失败！");
		}

		char bufWrite[1024];
		memset(bufWrite,0,sizeof(bufWrite));
		sprintf(bufWrite,"线程ID		时间	 				数据\n");

		fwrite(bufWrite,1,strlen(bufWrite),pFile);
		memset(bufWrite,0,sizeof(bufWrite));
		fflush(pFile);
		fclose(pFile);
	}

	FILE* fd;
	fd = fopen(logPath, "a");

	while(fd == NULL){
		fd = fopen(logPath, "a");
	}

	time_t t2 = time(0); 
	strftime( strTime, sizeof(strTime), "%Y/%m/%d %X",localtime(&t2) );	//得到当前时间
	//strftime( strTime, sizeof(strTime), "%Y/%m/%d %X %A 本年第%j天 %z",localtime(&t) );

	char bufWrite[260];
	memset(bufWrite,0,sizeof(bufWrite));

	std::string strWrite;
	//当前线程id
#ifdef WIN32
	int threadID  = GetCurrentThreadId();	
	sprintf(bufWrite,"%d		%s		",threadID,strTime);
	strWrite = bufWrite;
	strWrite += msg;
	strWrite+="\n";
#else
	sprintf(bufWrite,"%u		%s		",(unsigned int)syscall(SYS_gettid),strTime);
	strWrite = bufWrite;
	strWrite += msg;
	strWrite+="\n";
#endif;

	fwrite(strWrite.c_str(),1,strWrite.size(),fd);
	fflush(fd);
	fclose(fd);
	
	return 0;
}

int CLogData::WriteLog(const string &fileName ,const string &msg,const int operateCode /* = 0 */)
{
	DirIsExits();//创建目录

	if (fileName.empty())
	{
		return -1;
	}

	time_t t = time(0); 
	char strTime[64]; 
	strftime( strTime, sizeof(strTime), "%Y-%m-%d",localtime(&t) );	//年月日
	string strPathName;
	strPathName =".//log//logData//";
	strPathName+=strTime;
	strPathName+="-";
	strPathName+=fileName;
	strPathName+=".log";

	//在这里设置写入锁，防止同时写入文件
	MUTEX_LOCK(LogDataLock);
	int iRet = access(strPathName.c_str(),0);
	if (iRet == -1)
	{
		FILE *pFile;
		pFile = fopen(strPathName.c_str(), "a");
		if (pFile == NULL)
		{
			perror("文件打开失败！");
		}

		char bufWrite[1024];
		memset(bufWrite,0,sizeof(bufWrite));
		sprintf(bufWrite,"线程ID		时间	 				数据\n");

		fwrite(bufWrite,1,strlen(bufWrite),pFile);
		memset(bufWrite,0,sizeof(bufWrite));
		fflush(pFile);
		fclose(pFile);
	}

	FILE* fd;
	fd = fopen(strPathName.c_str(), "a");

	while(fd == NULL){
		fd = fopen(strPathName.c_str(), "a");
	}

	time_t t2 = time(0); 
	strftime(strTime, sizeof(strTime), "%Y/%m/%d %X",localtime(&t2) );	//得到当前时间
	//strftime( strTime, sizeof(strTime), "%Y/%m/%d %X %A 本年第%j天 %z",localtime(&t) );

	char bufWrite[260];
	memset(bufWrite,0,sizeof(bufWrite));

	std::string strWrite;
	//当前线程id
#ifdef WIN32
	int threadID  = GetCurrentThreadId();	
	sprintf(bufWrite,"%d		%s		",threadID,strTime);
	strWrite = bufWrite;
	strWrite += msg;
	strWrite+="\n";
#else
	sprintf(bufWrite,"%u		%s		",(unsigned int)syscall(SYS_gettid),strTime);
	strWrite = bufWrite;
	strWrite += msg;
	strWrite+="\n";
#endif;

	fwrite(strWrite.c_str(),1,strWrite.size(),fd);
	fflush(fd);
	fclose(fd);

	return 0;
}

int CLogData::WriteLogFormat(const string &fileName, ...) {

	va_list ap;
	va_start(ap, fileName);
	char *sz = NULL;
	string strLog = "";
	while((sz = va_arg(ap, char*)) != NULL) {
		strLog += "[";
		strLog += sz;
		strLog += "] ";
	}
	va_end(ap);
	return WriteLog(fileName, strLog);
}
#ifdef WIN32
void CLogData::TcharToChar (const TCHAR * tchar, char * _char)   
{   
	int iLength ; 
	//获取字节长度    
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);   
	//将tchar值赋给_char      
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);    
}  

//同上    
void CLogData::CharToTchar (const char * _char, TCHAR * tchar)   
{   
	int iLength ;   
	iLength = MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, NULL, 0) ;   
	MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, tchar, iLength) ;   
} 
#endif
