#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include "HandlerMessage.h"
using namespace std;
class ClientSocketServer {
public:
	ClientSocketServer(void);
	~ClientSocketServer(void);
	static DWORD WINAPI ThreadProcServer(PVOID pParam);
	BOOL SendMessage(int type, string strMessage);
	BOOL SendStoreCode();
	BOOL SendHeartbeat();
	BOOL Run();
	BOOL ResumeServer();
	BOOL SuspendServer();
	BOOL RestartServer();
	VOID WaitServerThread();
	VOID SetSocket(SOCKET clientSocket);
	VOID SetError(const int code, const char* msg);
	VOID SetError(const char* msg);
	VOID TerminateServer();

	static ClientSocketServer* getInstance();
	static void destoryInstance();
	CStringA m_serverIp;
	CStringA m_storehouseCode;
	int m_serverPort;
	BOOL m_bServerRun;
	static ClientSocketServer *m_instance;
private:
	BOOL initConfig();
	HANDLE m_serverHandle;
	DWORD m_serverThreadId;
	SOCKET m_clientSocket;

	
	BOOL m_bConfig;
};

