#pragma once
#include <iostream>
#include "socket_message.h"
#include <ClientSocketServer.h>
using namespace std;
class ClientSocketServer;
class CHandlerMessage
{
public:
	CHandlerMessage(void);
	CHandlerMessage(ClientSocketServer *socketServer, SOCKET_MESSAGE_HEADER msgHeader, string strData);
	~CHandlerMessage(void);
	static DWORD WINAPI ThreadProcHandler(PVOID pParam);
	BOOL Run();

	SOCKET_MESSAGE_HEADER m_msg_header;
	string m_str_data;
	ClientSocketServer *m_socket_server;
private:
	HANDLE m_handler_thread;
	DWORD m_handler_thread_id;
	
	
};

