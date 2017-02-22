
// wmsclientDlg.h : 头文件
//

#pragma once
#include "Resource.h"
#include "ClientSocketServer.h"
#include "afxwin.h"

#define PROCESS_EVENT_NAME "tx_datasync_event"
#define TIMER_HEART_BEAT 1001
#define TIMER_SERVER_HEART_BEAT 1002
// CwmsclientDlg 对话框
class CwmsclientDlg : public CDialogEx
{
// 构造
public:
	CwmsclientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WMSCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
