#ifndef _CLogData_H
#define _CLogData_H

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

#include <iostream>

#ifdef WIN32
#include  <io.h>
#include <process.h>
#include <Windows.h>
#include <shlobj.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include<unistd.h>
#include<sys/syscall.h>
#endif

#define MAX_PATH_SIZE 260

using namespace std;

class CLogData
{
public:
	CLogData(void);
	~CLogData(void);

public:
	static CLogData* getInstance();

	void DirIsExits();//判断目录是否存在，不存在就创建

	int WriteLog(const string &msg/*错误信息*/,const int operateCode = 0/*备用，默认值0*/); 
	int WriteLog(const string &fileName /*文件名称*/,const string &msg/*错误信息*/,const int operateCode = 0/*备用，默认值0*/); 
	int WriteLogFormat(const string &fileName, ...);
	
#ifdef WIN32
	void TcharToChar (const TCHAR * tchar, char * _char) ;
	void CharToTchar (const char * _char, TCHAR * tchar) ;
	#endif

private:
	static CLogData* m_Instance;

	class CDelete	/*释放内存*/
	{
		CDelete(void)
		{
		}

		~CDelete(void)
		{
			if(CLogData::m_Instance)
			{
				delete CLogData::m_Instance;
			}
		}
	};

	static CDelete m_delete;
};
#endif//_CLogData_H

