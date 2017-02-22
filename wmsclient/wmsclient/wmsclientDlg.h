
// wmsclientDlg.h : ͷ�ļ�
//

#pragma once
#include "Resource.h"
#include "ClientSocketServer.h"
#include "afxwin.h"

#define PROCESS_EVENT_NAME "tx_datasync_event"
#define TIMER_HEART_BEAT 1001
#define TIMER_SERVER_HEART_BEAT 1002
// CwmsclientDlg �Ի���
class CwmsclientDlg : public CDialogEx
{
// ����
public:
	CwmsclientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_WMSCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonBinlog();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnClose();

private:
	BOOL checkProcessIsAlready();
	ClientSocketServer *m_serverInstance;
	BOOL StartClientAll();
	BOOL m_query_db_run;
	bool StarBoot();
	bool alreadyStarBoot();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btnSetStart;
	afx_msg void OnBnClickedBtnSet();
	afx_msg void OnBnClickedBtnThreaStar();
	afx_msg void OnBnClickedBtnThreadEnd();
};
