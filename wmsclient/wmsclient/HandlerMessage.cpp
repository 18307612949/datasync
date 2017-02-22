#include "StdAfx.h"
#include "HandlerMessage.h"
#include "StringCoding.h"
#include "BuildFinally.h"
#include "MysqlOperation.h"
#include <MyException.h>
#include <LogData.h>
#include <RefreshUI.h>
#include <ClientSocketServer.h>
#include <UpdateDbThread.h>

CHandlerMessage::CHandlerMessage(void) {
}


CHandlerMessage::~CHandlerMessage(void) {
}
CHandlerMessage::CHandlerMessage(ClientSocketServer *socketServer, SOCKET_MESSAGE_HEADER msgHeader, string strData):m_socket_server(socketServer), m_msg_header(msgHeader),m_str_data(strData){
	m_str_data = StringCoding(strData).Utf82Gbk();
}

DWORD CHandlerMessage::ThreadProcHandler(LPVOID pParam) {
	CHandlerMessage *pInstance = (CHandlerMessage*)pParam;
	SOCKET_MESSAGE_HEADER &headerData = pInstance->m_msg_header;
	switch(headerData.code_) {
		case MSG_TYPE_UPDATE_DATABASE:{
			pInstance->m_socket_server->SendHeartbeat();
			g_instance_CUpdateDbThread.ProductData(pInstance->m_msg_header, pInstance->m_str_data);
			break;
		}
		case MSG_TYPE_HEART_BEAT: {
			
			if(pInstance->m_str_data.compare("begin") == 0) {
				pInstance->m_socket_server->SendStoreCode();
			}
			else {
				pInstance->m_socket_server->SendHeartbeat();
			}
			break;
		}
		case MSG_TYPE_CONNECT_SUCCESS: {
			CRefreshUI *uiInstance = CRefreshUI::getInstance();
			uiInstance->ChangeConnectionState(TRUE);
			uiInstance->BeginSocketTimer(8000);
			
			//uiInstance->BeginSocketTimer();
			break;
		}
		default:{
				
				break;
		}
	}
	
	
	delete pInstance;
	return 0;
}

BOOL CHandlerMessage::Run(){
	CRefreshUI *uiInstance = CRefreshUI::getInstance();
	uiInstance->SetHeartbeatTimestamp(time(0));
	m_handler_thread = CreateThread(NULL, 0, ThreadProcHandler, this, CREATE_SUSPENDED, &m_handler_thread_id);
	if(m_handler_thread == NULL)
		return FALSE;
	ResumeThread(m_handler_thread);
	return TRUE;
}
