
// wmsclient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CwmsclientApp:
// �йش����ʵ�֣������ wmsclient.cpp
//

class CwmsclientApp : public CWinApp
{
public:
	CwmsclientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CwmsclientApp theApp;