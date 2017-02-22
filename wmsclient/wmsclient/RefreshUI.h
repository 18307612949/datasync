#pragma once
#include "wmsclientDlg.h"
#include "defines.h"
class CRefreshUI
{
public:
	CRefreshUI(void);
	CRefreshUI(CwmsclientDlg *pDlg);
	~CRefreshUI(void);
	static CRefreshUI* getInstance(CwmsclientDlg *pDlg = NULL);
	static CRefreshUI* m_instance;
	VOID ChangeConnectionState(BOOL isConnection);
	BOOL ChangeConnectionState();
	void initShowDlg();
	VOID SetHeartbeatTimestamp(long long tm);
	VOID BeginSocketTimer(int senconds = 2000);
	VOID StopSocketTimer();
private:
	CwmsclientDlg* m_mainDlgInstance;
	long long m_lastHeartbeatTimestamp;
};

