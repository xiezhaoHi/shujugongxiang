
// WPD_MTP_data.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "WPD_MTP_dataDlg.h"
#include "startExeDlg.h"
#include "MytestDlg.h"
#include "verity/MyVerify.h"
#include "LogDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWPD_MTP_dataApp

BEGIN_MESSAGE_MAP(CWPD_MTP_dataApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWPD_MTP_dataApp ����

CWPD_MTP_dataApp::CWPD_MTP_dataApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CWPD_MTP_dataApp ����

CWPD_MTP_dataApp theApp;


// CWPD_MTP_dataApp ��ʼ��

BOOL CWPD_MTP_dataApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	//1.��־��ʼ��
	if (!CLogRecord::InitLogRecord())
	{
		AfxMessageBox(_T("��־��ʼ��ʧ��,error:��������[logfiles]�ļ���!"));
		return FALSE;
	}
	CreateEvent(nullptr, 0, 0, _T("4f562239-2d0e-4bb0-b0ac-ad1fee1be174"));
	int err = GetLastError();
	if (183 == err)
	{
		CLogRecord::WriteRecordToFile(_T("����ظ�����,ʧ��!"));
#ifndef DEBUG 
		return FALSE;
#endif
	}
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	if (!SUCCEEDED(hr))
	{
		AfxMessageBox(_T("COM��ʼ��ʧ��"));
		return FALSE;
	}

	//�û������֤
	INT_PTR ret ;
	CString strUserArea,strUserName;
#ifndef DEBUG
	if (!MyVerify::GetInstance()->VerityExe())
	{
		CstartExeDlg startDlg;

		ret = startDlg.DoModal();
	}
	else
	{
		ret = IDOK;
	}
	if (ret != IDOK)
	{
		CLogRecord::WriteRecordToFile(_T("�����֤ʧ��,�û�δע��!"));
		AfxMessageBox(_T("�����֤ʧ��,�û�δע��!"));
		return FALSE;
	}
	CLogDlg logDlg;
	 ret = logDlg.DoModal();

	if (ret != IDOK)
	{
		CLogRecord::WriteRecordToFile(_T("ȡ����¼!"));
		AfxMessageBox(_T("�û���¼ʧ��,ȡ����¼!"));
		return FALSE;
	}
	strUserArea = logDlg.GetUserAreas();
	strUserName = logDlg.GetUserRealName();
#endif
	

	CLogRecord::WriteRecordToFile(_T("--------------��������1.0.5.0!--------------"));
	CWPD_MTP_dataDlg dlg;
	dlg.SetUserAreaRealName(strUserArea,strUserName);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}
	//�ͷ�COM
	CoUninitialize();

	CLogRecord::WriteRecordToFile(_T("--------------�������!--------------"));

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

