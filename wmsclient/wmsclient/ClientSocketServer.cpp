#include "StdAfx.h"
#include "ClientSocketServer.h"
#include <WinSock.h>
#include "defines.h"
#include "StringCoding.h"
#include "socket_message.h"
#include "lock.h"
#include <MyException.h>
#include <LogError.h>
#include <RefreshUI.h>
#include <Commfun.h>
#pragma comment(lib,"ws2_32.lib")

ClientSocketServer* ClientSocketServer::m_instance = NULL;
ClientSocketServer::ClientSocketServer(void) {
	m_serverHandle = NULL;
	m_serverThreadId = 0;
	m_clientSocket = INVALID_SOCKET;
	initConfig();
	m_bServerRun = TRUE;
}


ClientSocketServer::~ClientSocketServer(void) {
	if(m_serverHandle != NULL)
		CloseHandle(m_serverHandle);
	if(m_clientSocket != INVALID_SOCKET)
		closesocket(m_clientSocket);
}

ClientSocketServer* ClientSocketServer::getInstance(){
	if(m_instance == NULL) {
		MUTEX_LOCK(ClientSocketServer);
		if(m_instance == NULL)
			m_instance = new ClientSocketServer();
	}
	return m_instance;
}

void ClientSocketServer::destoryInstance() {
	MUTEX_LOCK(ClientSocketServer);
	if(ClientSocketServer::m_instance != NULL) {
		delete ClientSocketServer::m_instance;
		ClientSocketServer::m_instance = NULL;
	}
}
BOOL ClientSocketServer::initConfig(){
	CFileFind finder;
	BOOL ifFind = finder.FindFile(_T(CONFIG_PATH));
	if(ifFind == FALSE){
		THROW_EXCEPTION(ExceptionSocket, "config file is not exist");
		return FALSE;
	}
	DWORD iRv;
	iRv = ::GetPrivateProfileStringA(SOCKET_CONFIG_KEY, "ServerIp","",m_serverIp.GetBuffer(MAX_PATH),MAX_PATH,CONFIG_PATH);
	if(iRv == 0) {
		THROW_EXCEPTION(ExceptionSocket, "config read [ServerIp] failed");
		return FALSE;
	}
	m_serverPort = ::GetPrivateProfileIntA(SOCKET_CONFIG_KEY, "ServerPort", 0, CONFIG_PATH);
	if(m_serverPort == 0) {
		THROW_EXCEPTION(ExceptionSocket, "config read [ServerPort] failed");
		return FALSE;
	}
	iRv = ::GetPrivateProfileStringA(STOREHOUSE_CONFIG_KEY, "Code","",m_storehouseCode.GetBuffer(MAX_PATH),MAX_PATH,CONFIG_PATH);
	if(iRv == 0) {
		THROW_EXCEPTION(ExceptionSocket, "config read [Code] failed");
		return FALSE;
	}
	return TRUE;
}

VOID ClientSocketServer::SetSocket(SOCKET clientSocket) {
	m_clientSocket = clientSocket;
}

VOID ClientSocketServer::SetError(const int code, const char* msg) {
	
}

VOID ClientSocketServer::SetError(const char* msg) {
	CLogError::getInstance()->WriteErrorLog(WSAGetLastError(), msg);
}
VOID ClientSocketServer::TerminateServer(){
	m_bServerRun = FALSE;
	Sleep(2000);
	TerminateThread(m_serverHandle, 100);
	WaitForSingleObject(m_serverHandle, 3000);
	OutputDebugStringA("wait obj");
	CloseHandle(m_serverHandle);
}

DWORD WINAPI ClientSocketServer::ThreadProcServer(PVOID pParam) {
	ClientSocketServer* pInstance = (ClientSocketServer*)pParam;

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if(WSAStartup(sockVersion, &wsaData) != 0) {
		pInstance->SetError("WSAStartup failed");
		goto error;
	}
	SOCKET socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socketClient == INVALID_SOCKET) {
		WSACleanup();
		pInstance->SetError("socket create failed");
		goto error;
	}
	sockaddr_in socketAddr;
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(pInstance->m_serverPort);
//	socketAddr.sin_addr.S_un.S_addr = inet_addr("192.168.89.240");
	socketAddr.sin_addr.S_un.S_addr = inet_addr(pInstance->m_serverIp.GetBuffer(0));
	int iOuttime = 3000;
	setsockopt(socketClient, SOL_SOCKET, SO_SNDTIMEO, (const char*)(&iOuttime), sizeof(iOuttime));
//	setsockopt(socketClient, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&iOuttime), sizeof(iOuttime));
	
	for(int nsec=1; ; nsec <<=1) {
		if(connect(socketClient, (sockaddr*)&socketAddr, sizeof(socketAddr)) == SOCKET_ERROR) {
			if(nsec >= 32){
				nsec = 1;
			}
			pInstance->SetError("socket connect failed");
			Sleep(nsec*1000);
		}
		else {
			break;
		}
	}

	pInstance->SetSocket(socketClient);
	pInstance->m_bServerRun = TRUE;
	while(pInstance->m_bServerRun){
		char buf[MESSAGE_BUFFER_OVER_LEN + 1];
		char *pBuf = NULL;
		memset(buf, 0x00, MESSAGE_BUFFER_OVER_LEN + 1);
		int recvLen = 0;
		SOCKET_MESSAGE_HEADER msgHeader;
		if((recvLen = mRecv(socketClient, (char*)&msgHeader, sizeof(SOCKET_MESSAGE_HEADER))) > 0) {
			int bodyLen = msgHeader.data_len_ - sizeof(SOCKET_MESSAGE_HEADER);
			pBuf = (char*)malloc((bodyLen + 1) * sizeof(char));
			if(pBuf == NULL){
				break;
			}
			memset(pBuf, 0x00, bodyLen + 1);
			//recvLen = recv(socketClient, pBuf, bodyLen, 0);
			recvLen = mRecv(socketClient, pBuf, bodyLen);
			if(recvLen != bodyLen) {
				free(pBuf);
#ifdef LOG_DEBUG
				char szTmp[1024] = {0};
				sprintf(szTmp, "body data recv failed[messageid=%s,bodylen=%d,recvlen=%d]", msgHeader.message_id_, bodyLen, recvLen);
				pInstance->SetError(szTmp);
#endif
				break;
			}
			recvLen = mRecv(socketClient, buf, MESSAGE_BUFFER_OVER_LEN);
			if(recvLen != MESSAGE_BUFFER_OVER_LEN){
				free(pBuf);
#ifdef LOG_DEBUG
				char szTmp[1024] = {0};
				sprintf(szTmp, "MESSAGE_BUFFER_OVER recv failed[messageid=%s,bodylen=%d,recvlen=%d, recvdata=%s]", msgHeader.message_id_, bodyLen, recvLen, buf);
				pInstance->SetError(szTmp);
#endif
				continue;
			}
			if(strcmp(buf, MESSAGE_BUFFER_OVER) != 0) {
				free(pBuf);
#ifdef LOG_DEBUG
				char szTmp[1024] = {0};
				sprintf(szTmp, "MESSAGE_BUFFER_OVER is not cmp[messageid=%s,bodylen=%d,recvlen=%d, recvdata=%s]", msgHeader.message_id_, bodyLen, recvLen, buf);
				pInstance->SetError(szTmp);
#endif
				continue;
			}
			CHandlerMessage *pHandlerInstance = new CHandlerMessage(pInstance, msgHeader, pBuf);
			pHandlerInstance->Run();
			free(pBuf);
		}
	}
error:
	closesocket(socketClient);
	WSACleanup();
	return 0;
}

BOOL ClientSocketServer::Run(){
	m_serverHandle = CreateThread(NULL, 0, ThreadProcServer, this, CREATE_SUSPENDED, &m_serverThreadId);
	if(m_serverHandle == NULL) {
		CLogError::getInstance()->WriteErrorLog(::GetLastError(), "create server thread failed");
		THROW_EXCEPTION(ExceptionSocket, "create server thread failed");
		return FALSE;
	}
	ResumeThread(m_serverHandle);
//	CloseHandle(m_serverHandle);
	return TRUE;
}
BOOL ClientSocketServer::RestartServer(){
	m_bServerRun = FALSE;
	return TRUE;
}
BOOL ClientSocketServer::SendMessage(int type, string strMessage) {
	if(m_clientSocket == INVALID_SOCKET) {
		SetError("socket handle INVALID_SOCKET");
		return FALSE;
	}
	OutputDebugStringA("MSG_TYPE_HEART_BEAT");
	strMessage = StringCoding(strMessage).Gbk2Utf8();
	int iSendLen = strMessage.length() + sizeof(SOCKET_MESSAGE_HEADER);
	SOCKET_MESSAGE_HEADER msgData;
	msgData.code_ = type;
	msgData.data_len_ = iSendLen;
	char* pSendBuf = (char*)malloc(iSendLen);
	memset(pSendBuf, 0x00, iSendLen);
	memcpy(pSendBuf, &msgData, sizeof(SOCKET_MESSAGE_HEADER));
	memcpy(pSendBuf+sizeof(SOCKET_MESSAGE_HEADER), strMessage.c_str(), strMessage.length());
	MUTEX_LOCK(ClientSocketServer);
	if(mSend(m_clientSocket, (const char*)pSendBuf, iSendLen) != iSendLen) {
		free(pSendBuf);
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
		SetError("socket send failed");
		return FALSE;
	}
	free(pSendBuf);
	if(mSend(m_clientSocket, MESSAGE_BUFFER_OVER, strlen(MESSAGE_BUFFER_OVER)) != strlen(MESSAGE_BUFFER_OVER)) {
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
		SetError("socket send failed");
		return FALSE;
	}
	return TRUE;
}

BOOL ClientSocketServer::SendStoreCode(){
	return SendMessage(MSG_TYPE_STORECODE, m_storehouseCode.GetBuffer(0));
}

BOOL ClientSocketServer::SendHeartbeat(){
	if(m_clientSocket == INVALID_SOCKET) {
		return FALSE;
	}
	return SendMessage(MSG_TYPE_HEART_BEAT, "heartbeat");
}

VOID ClientSocketServer::WaitServerThread() {
	::WaitForSingleObject(m_serverHandle, INFINITE);
}