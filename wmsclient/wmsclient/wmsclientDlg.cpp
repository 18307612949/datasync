
// wmsclientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "wmsclient.h"
#include "wmsclientDlg.h"
#include "afxdialogex.h"
#include "../lib/BinlogMonitor.h"
#include <WinSock.h>
#include <mysql.h>
#include <lock.h>
#include "RefreshUI.h"
#include <binlog/export.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma comment(lib,"binlog.lib")
#pragma comment(lib, "libmysql.lib")

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CwmsclientDlg �Ի���




CwmsclientDlg::CwmsclientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CwmsclientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_serverInstance = NULL;
	m_query_db_run = FALSE;
}

void CwmsclientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK, m_btnSetStart);
}

BEGIN_MESSAGE_MAP(CwmsclientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BINLOG, &CwmsclientDlg::OnBnClickedButtonBinlog)
	ON_BN_CLICKED(IDC_BUTTON2, &CwmsclientDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_SET, &CwmsclientDlg::OnBnClickedBtnSet)
	ON_BN_CLICKED(IDC_BTN_THREA_STAR, &CwmsclientDlg::OnBnClickedBtnThreaStar)
	ON_BN_CLICKED(IDC_BTN_THREAD_END, &CwmsclientDlg::OnBnClickedBtnThreadEnd)
END_MESSAGE_MAP()


// CwmsclientDlg ��Ϣ�������

BOOL CwmsclientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	char szModule[MAX_PATH]={0};
	GetModuleFileNameA(NULL,szModule,MAX_PATH);
	string sTemp = szModule;
	string sub =  sTemp.substr(0,sTemp.rfind('\\')+1);
	SetCurrentDirectoryA(sub.c_str());

	if(checkProcessIsAlready() == FALSE) {
		initMutex();
		LoadBinlogDll();
		CRefreshUI::getInstance(this)->initShowDlg();
		StartClientAll();
	}
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}
BOOL CwmsclientDlg::checkProcessIsAlready(){return FALSE;
	HANDLE hProcessHandle = CreateEvent(NULL, TRUE, TRUE, _T(PROCESS_EVENT_NAME));
	if(hProcessHandle != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(_T("����ͬ���ͻ���������"), _T("����"), MB_OK);
		this->PostMessage(WM_CLOSE);
		return TRUE;
	}
	return FALSE;
}

void CwmsclientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CwmsclientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CwmsclientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


DWORD WINAPI monitorThread(LPVOID lpParamter) {
//	CParseLogEntry *pInstance = CParseLogEntry::getInstance();
	return 0;
}
/*
DWORD WINAPI func(LPVOID pParam) {
	for()
}*/
#include "PreBinlog.h"
#include "MysqlOperation.h"

#include "socket_message.h"
#include <json\json.h>
#include <iostream>
#include <vector>
using namespace std;
#include <MyException.h>
#include <LogError.h>
#include <LogData.h>
#include <UpdateDbThread.h>
#include <BuildFinally.h>
BOOL CwmsclientDlg::StartClientAll(){
//	string s = "{\"tables\":[{\"name\":\"customers\",\"column\":[\"AREA_ID\",\"X_COOR\",\"CONTACT\",\"C_NAME\",\"C_CODE\",\"PHONE\",\"ADDRESS\",\"Y_COOR\",\"WH_CODE\"],\"values\":[\"92168\",\"103.938093\",\"�δ���\",\"�δ���ʳ��\",\"322670\",\"13111898611\",\"�Ĵ��Ĵ��ɶ��������º����º������ִ��\��ȪС����\",\"30.950465\",\"260\"],\"mapkey\":{\"C_CODE\":322670}}]}";
//	CBuildFinally p(s);
//	p.LoadJson();

	try{
		CMysqlOperation *p = CMysqlOperation::getInstance();
		m_serverInstance = ClientSocketServer::getInstance();
		m_serverInstance->Run();
		g_instance_CUpdateDbThread.Run();
		CPreBinlog::getInstance()->StartProcessBinlog();
	}
	catch(ExceptionDatabase &e){
		MessageBoxA(NULL, e.what(), "", 0);
	}
	catch(ExceptionSocket &e){
		MessageBoxA(NULL, e.what(), "", 0);
	}
	catch(ExceptionBase &e) {
		MessageBoxA(NULL, e.what(), "", 0);
	}
	return TRUE;
}
void CwmsclientDlg::OnBnClickedButtonBinlog()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	initMutex();
//	string s = "E://log//mysql";
	/*
	CBinlogMonitor* pc  = new CBinlogMonitor("E://log//mysql");
	pc->startMonitor();
	CTest pInstance("tangxin");
	MessageBoxA(NULL, pInstance.getSz().c_str(), "ddd", 0);
	CreateThread(NULL, 0, monitorThread, pc, 0, NULL);
	*/
//	pInstance.startMonitor();
	ParseBinlogThread("G://log//mysql-bin.000002");
//	while(true)
//		CRowData *pData = consumerRowData();

//	int i = 0;
	CPreBinlog::getInstance()->StartProcessBinlog();

}

void CwmsclientDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	try{
		CMysqlOperation *p = CMysqlOperation::getInstance();
		m_serverInstance = ClientSocketServer::getInstance();
		m_serverInstance->Run();
		g_instance_CUpdateDbThread.Run();
	}
	catch(ExceptionDatabase &e){
		MessageBoxA(NULL, e.what(), "", 0);
	}
	catch(ExceptionSocket &e){
		MessageBoxA(NULL, e.what(), "", 0);
	}
	catch(ExceptionBase &e) {
		MessageBoxA(NULL, e.what(), "", 0);
	}
}


void CwmsclientDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	KillTimer(TIMER_SERVER_HEART_BEAT);
	destoryMutex();
	UnloadBinlogDll();
	CDialogEx::OnClose();
}


void CAboutDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// TODO: �ڴ˴������Ϣ����������
}


void CwmsclientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch(nIDEvent){
		case TIMER_SERVER_HEART_BEAT: {
			if(CRefreshUI::getInstance()->ChangeConnectionState() == FALSE){
				CRefreshUI::getInstance()->StopSocketTimer();
				m_serverInstance->TerminateServer();
				m_serverInstance->Run();

				//if (m_query_db_run)
				//{
				//	m_query_db_run = FALSE;
				//	StopGenerateData();//�ر�ȡ��������
				//}
			}
			else
			{
				//if (m_query_db_run == FALSE)
				//{
				//	m_query_db_run = TRUE;
				//	StartGenerateData();//����ȡ��������
				//}
			}
			break;
		}
		default:{
				
				break;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

//����Ƿ񿪻�����
bool CwmsclientDlg::alreadyStarBoot()
{
	HKEY hOpen;
	char *szSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";//HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node
	//char *szSubKey = "Software\\Wow6432Node\\Windows\\CurrentVersion\\Run";
	char szModule[MAX_PATH]={0};
	GetModuleFileNameA(NULL,szModule,MAX_PATH);
	string sTemp = szModule;
	string sub =  sTemp.substr(sTemp.rfind('\\')+1,sTemp.size());
	string subStr = sub.substr(0,sub.rfind('.'));

	char buf[MAX_PATH]={0};
	DWORD size;
	if(ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCWSTR)szSubKey, 0, KEY_READ, &hOpen)) {
		RegQueryValueExA(hOpen, subStr.c_str(), NULL, NULL, (BYTE*)buf, &size);
	}
	RegCloseKey(hOpen);
	if (strlen(buf) > 0 )
	{
		return true;
	}

	return false;
}

//��ӿ�����������
bool CwmsclientDlg::StarBoot()
{
	HKEY hRoot = HKEY_LOCAL_MACHINE;
	HKEY hOpen;
	char *szSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	//char *szSubKey = "Software\\Wow6432Node\\Windows\\CurrentVersion\\Run";
	char szModule[MAX_PATH]={0};
	GetModuleFileNameA(NULL,szModule,MAX_PATH);
	string sTemp = szModule;
	string sub =  sTemp.substr(sTemp.rfind('\\')+1,sTemp.size());
	string subStr = sub.substr(0,sub.rfind('.'));

	HKEY hKey;
	DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
	//RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCWSTR)szSubKey, 0, KEY_READ|KEY_WRITE|KEY_WOW64_64KEY, &hOpen);
	long lRet = RegCreateKeyExA(hRoot,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL,&hKey,&dwDisposition);
	if (lRet!= ERROR_SUCCESS)
	{
		return false;
	}

	lRet = RegSetValueExA(hKey,subStr.c_str(),0,REG_SZ,(BYTE *)szModule,strlen(szModule));
	RegCloseKey(hKey);

	if (lRet != ERROR_SUCCESS)
	{
		return false;
	}

	return true;
}

void CwmsclientDlg::OnBnClickedBtnSet()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_btnSetStart.GetCheck())
	{
		//�Ƿ��Ѿ���������
		bool bRet = alreadyStarBoot();
		if(!bRet)
		{//û�п�����������ӿ�������
			bRet = StarBoot();
		}
	}
}


void CwmsclientDlg::OnBnClickedBtnThreaStar()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_query_db_run == FALSE)
	{
		m_query_db_run = TRUE;
		StartGenerateData();//����ȡ��������

		GetDlgItem(IDC_BTN_THREA_STAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_THREAD_END)->EnableWindow(TRUE);
	}
}


void CwmsclientDlg::OnBnClickedBtnThreadEnd()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_query_db_run)
	{
		m_query_db_run = FALSE;
		StopGenerateData();//�ر�ȡ��������
		GetDlgItem(IDC_BTN_THREA_STAR)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_THREAD_END)->EnableWindow(FALSE);
	}
}
