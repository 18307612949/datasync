#pragma once
#include <iostream>
#include <deque>
#include <map>
using namespace std;
#include "socket_message.h"
#define ROWS_DATA_MAX_SIZE 1000
typedef struct update_data{
	SOCKET_MESSAGE_HEADER header_;
	string body_;
}UPDATE_DATA;
class CUpdateDbThread
{
public:
	CUpdateDbThread(void);
	~CUpdateDbThread(void);
	static DWORD WINAPI ThreadProcess(PVOID pParam);
	BOOL Run();
	VOID ProductData(SOCKET_MESSAGE_HEADER header, const string& strData);
	BOOL ConsumerData(UPDATE_DATA &data);
private:
	deque<UPDATE_DATA> m_queue_data;
	HANDLE m_handler_thread;
	DWORD m_handler_thread_id;
};

extern CUpdateDbThread g_instance_CUpdateDbThread;