#include "LogError.h"
#include "lock.h"

int glb_MessageQueue_read_success = 0;
int glb_MessageQueue_write_success = 0;
int glb_sock_send_success = 0;
int glb_sock_recv_success = 0;
        
//在此处初始化
CLogError* CLogError::m_Instance = NULL;
//int LogErrorLock;

CLogError::CLogError()
{
	MakeDir();//创建目录
}


CLogError::~CLogError(void)
{
}

CLogError* CLogError::getInstance()
{
	//很多线程的话，就爱会造成大量线程的阻塞,这样写降低了线程的阻塞的几率
	
	if(m_Instance == NULL)
	{
		MUTEX_LOCK(LogErrorLock);	
		if (m_Instance == NULL)
		{
			m_Instance = new CLogError();
		}
	}

	return m_Instance;
}

void CLogError::MakeDir()
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
		sprintf(strDir,"%s\\log\\logError",sub.c_str());
		TCHAR tcreate[MAX_PATH]={0};
		CharToTchar(strDir,tcreate);
		SHCreateDirectory(0,tcreate);
		//system(strDir);
#else
		char strPath[253]="mkdir log";
		char strChild[64]="mkdir log//logError";
		system(strPath);
		system(strChild);
#endif
	}
	else/*目录存在*/
	{	
		char strPathChild[253]="log//logError";
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
			sprintf(strDir,"%s\\log\\logError",sub.c_str());
			TCHAR tcreate[MAX_PATH]={0};
			CharToTchar(strDir,tcreate);
			SHCreateDirectory(0,tcreate);
#else
			char strChild[64]="mkdir log//logError";
			system(strChild);
#endif
		}
	}
}

int CLogError::WriteErrorLog(const int code,const string &msgError)
{
	MakeDir();//创建目录

	time_t t = time(0); 
	char strTime[64]; 
	strftime( strTime, sizeof(strTime), "%Y-%m-%d",localtime(&t) );	//年月日
	char logPath[MAX_PATH_SIZE];
	memset(logPath,0,MAX_PATH_SIZE);
	sprintf(logPath,".//log//logError//%s-logError.log",strTime);

	
	//在这里设置写入锁，防止同时写入文件
	MUTEX_LOCK(LogErrorLock);

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
		sprintf(bufWrite,"线程ID		时间					错误码		错误信息\n");

		fwrite(bufWrite,1,strlen(bufWrite),pFile);
		memset(bufWrite,0,sizeof(bufWrite));
		fflush(pFile);
		fclose(pFile);
	}

	FILE* fd;
    fd = fopen(logPath, "a");
    if (fd == NULL)
	{
        perror("文件打开失败！");
	}

	time_t t2 = time(0); 
	
	strftime( strTime, sizeof(strTime), "%Y/%m/%d %X",localtime(&t2) );	//得到当前时间
	//strftime( strTime, sizeof(strTime), "%Y/%m/%d %X %A 本年第%j天 %z",localtime(&t) );
	
	int threadID;	//当前线程id
	char bufWrite[260];
	memset(bufWrite,0,sizeof(bufWrite));

	std::string strError;

#ifdef WIN32
	threadID  = GetCurrentThreadId();
	
	sprintf(bufWrite,"%d		%s		%d		",threadID,strTime,code);	//写入数据格式为线程ID、写入时间、错误码、错误信息
	strError = bufWrite;
	strError += msgError;
	strError += "\n";
#else
	sprintf(bufWrite,"%u		%s		%d		",(unsigned int)syscall(SYS_gettid),strTime,code);	//写入数据格式为线程ID、写入时间、错误码、错误信息
	strError = bufWrite;
	strError += msgError;
	strError += "\n";
#endif;

	fwrite(strError.c_str(),1,strError.size(),fd);
	fflush(fd);
	fclose(fd);

	return 0;
}

int CLogError::WriteErrorLog(const int code,const string &msgError,const string &fileName)
{
	MakeDir();//创建目录

	if (fileName.empty())
	{
		return -1;
	}

	time_t t = time(0); 
	char strTime[64]={0}; 
	strftime( strTime, sizeof(strTime), "%Y-%m-%d",localtime(&t) );	//年月日
	string strPathName;
	strPathName =".//log//logError//";
	strPathName+=strTime;
	strPathName+="-";
	strPathName+=fileName;
	strPathName+=".log";


	//在这里设置写入锁，防止同时写入文件
	MUTEX_LOCK(LogErrorLock);

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
		sprintf(bufWrite,"线程ID		时间					错误码		错误信息\n");

		fwrite(bufWrite,1,strlen(bufWrite),pFile);
		memset(bufWrite,0,sizeof(bufWrite));
		fflush(pFile);
		fclose(pFile);
	}

	FILE* fd;
	fd = fopen(strPathName.c_str(), "a");
	if (fd == NULL)
	{
		perror("文件打开失败！");
	}

	time_t t2 = time(0); 

	strftime( strTime, sizeof(strTime), "%Y/%m/%d %X",localtime(&t2) );	//得到当前时间
	//strftime( strTime, sizeof(strTime), "%Y/%m/%d %X %A 本年第%j天 %z",localtime(&t) );

	int threadID;	//当前线程id
	char bufWrite[260];
	memset(bufWrite,0,sizeof(bufWrite));

	std::string strError;

#ifdef WIN32
	threadID  = GetCurrentThreadId();

	sprintf(bufWrite,"%d		%s		%d		",threadID,strTime,code);	//写入数据格式为线程ID、写入时间、错误码、错误信息
	strError = bufWrite;
	strError += msgError;
	strError += "\n";
#else
	sprintf(bufWrite,"%u		%s		%d		",(unsigned int)syscall(SYS_gettid),strTime,code);	//写入数据格式为线程ID、写入时间、错误码、错误信息
	strError = bufWrite;
	strError += msgError;
	strError += "\n";
#endif;

	fwrite(strError.c_str(),1,strError.size(),fd);
	fflush(fd);
	fclose(fd);

	return 0;
}

#ifdef WIN32
void CLogError::TcharToChar (const TCHAR * tchar, char * _char)   
{   
	int iLength ; 
	//获取字节长度    
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);   
	//将tchar值赋给_char      
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);    
}  

//同上    
void CLogError::CharToTchar (const char * _char, TCHAR * tchar)   
{   
	int iLength ;   
	iLength = MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, NULL, 0) ;   
	MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, tchar, iLength) ;   
} 
#endif


