#include "StdAfx.h"
#include "RefreshUI.h"
#include <lock.h>
CRefreshUI* CRefreshUI::m_instance = NULL;
CRefreshUI::CRefreshUI(void):m_mainDlgInstance(NULL)
{
	m_lastHeartbeatTimestamp = 0;
}
CRefreshUI::CRefreshUI(CwmsclientDlg *pDlg):m_mainDlgInstance(pDlg){
	m_lastHeartbeatTimestamp = 0;
}

CRefreshUI::~CRefreshUI(void)
{
}

CRefreshUI* CRefreshUI::getInstance(CwmsclientDlg *pDlg) {
	if(m_instance == NULL){
		MUTEX_LOCK(RefreshUI);
		if(m_instance == NULL){
			m_instance = new CRefreshUI(pDlg);
		}
	}
	return m_instance;
}

VOID CRefreshUI::SetHeartbeatTimestamp(long long tm){
	m_lastHeartbeatTimestamp = tm;
}

void CRefreshUI::initShowDlg(){
	if(m_mainDlgInstance == NULL)
		return;
	MUTEX_LOCK(RefreshUI);
	CString csTmp;
	::GetPrivateProfileString(_T(STOREHOUSE_CONFIG_KEY), _T("Name"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_STORENAME)->SetWindowText(csTmp);

	::GetPrivateProfileString(_T(STOREHOUSE_CONFIG_KEY), _T("Code"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_STOREID)->SetWindowText(csTmp);

	::GetPrivateProfileString(_T(SOCKET_CONFIG_KEY), _T("ServerIp"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_SERVER_IP)->SetWindowText(csTmp);

	::GetPrivateProfileString(_T(SOCKET_CONFIG_KEY), _T("ServerPort"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_SERVER_PORT)->SetWindowText(csTmp);

	::GetPrivateProfileString(_T(DB_CONFIG_KEY), _T("Host"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_DATABASE_HOST)->SetWindowText(csTmp);

	::GetPrivateProfileString(_T(DB_CONFIG_KEY), _T("UserName"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_DATABASE_USERNAME)->SetWindowText(csTmp);

	::GetPrivateProfileString(_T(DB_CONFIG_KEY), _T("DatabaseName"), _T(""),csTmp.GetBuffer(MAX_PATH),MAX_PATH,_T(CONFIG_PATH));
	m_mainDlgInstance->GetDlgItem(IDC_EDIT_DATABASE_NAME)->SetWindowText(csTmp);

	m_mainDlgInstance->GetDlgItem(IDC_STATIC_SERVER_TIP)->SetWindowTextW(_T("正在连接中..."));

	m_mainDlgInstance->GetDlgItem(IDC_RADIO_SERVER_STATE)->EnableWindow(FALSE);
	((CButton*)m_mainDlgInstance->GetDlgItem(IDC_RADIO_SERVER_STATE))->SetCheck(FALSE);
}

VOID CRefreshUI::ChangeConnectionState(BOOL isConnection) {
	CString csShow = _T("已连接");
	CString csTip = _T("");
	if(isConnection == FALSE) {
		csShow = _T("未连接");
		csTip = _T("正在连接中...");
	}
	((CButton*)m_mainDlgInstance->GetDlgItem(IDC_RADIO_SERVER_STATE))->SetCheck(isConnection);
	m_mainDlgInstance->GetDlgItem(IDC_RADIO_SERVER_STATE)->SetWindowText(csShow);
	m_mainDlgInstance->GetDlgItem(IDC_STATIC_SERVER_TIP)->SetWindowTextW(csTip);
	
}

BOOL CRefreshUI::ChangeConnectionState(){
	long long newTm = time(0);
	if(newTm - m_lastHeartbeatTimestamp > 16){
		ChangeConnectionState(FALSE);
		return FALSE;
	}
	else {
		ChangeConnectionState(TRUE);
		return TRUE;
	}
}

VOID CRefreshUI::BeginSocketTimer(int senconds){
	m_mainDlgInstance->KillTimer(TIMER_SERVER_HEART_BEAT);
	m_mainDlgInstance->SetTimer(TIMER_SERVER_HEART_BEAT, senconds, NULL);
}

VOID CRefreshUI::StopSocketTimer(){
	m_mainDlgInstance->KillTimer(TIMER_SERVER_HEART_BEAT);
}