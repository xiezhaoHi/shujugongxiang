
// WPD_MTP_data.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CWPD_MTP_dataApp: 
// �йش����ʵ�֣������ WPD_MTP_data.cpp
//

class CWPD_MTP_dataApp : public CWinApp
{
public:
	CWPD_MTP_dataApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CWPD_MTP_dataApp theApp;