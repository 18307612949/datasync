#ifndef _CLogError_H
#define _CLogError_H
#include "stdio.h"
#include "time.h"
#include "string.h"
#include "stdlib.h"

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

extern int glb_MessageQueue_read_success;
extern int glb_MessageQueue_write_success;
extern int glb_sock_send_success;
extern int glb_sock_recv_success;

class CLogError
{
public:
	CLogError();
	~CLogError(void);

public:
	static CLogError* getInstance();

	void MakeDir();////判断目录是否存在，不存在就创建

	int WriteErrorLog(const int codeError/*错误码*/,const string &msgError/*错误信息*/); 
	int WriteErrorLog(const int code,const string &msgError,const string &fileName/*文件名称*/);

	#ifdef WIN32
	void TcharToChar (const TCHAR * tchar, char * _char) ;
	void CharToTchar (const char * _char, TCHAR * tchar) ;
	#endif

private:
	static CLogError* m_Instance;

	class CDelete	/*释放内存*/
	{
		CDelete(void)
		{
		}

		~CDelete(void)
		{
			if(CLogError::m_Instance)
			{
				delete CLogError::m_Instance;
			}
		}
	};

	static CDelete m_delete;
};

#endif//_CLogError_H


