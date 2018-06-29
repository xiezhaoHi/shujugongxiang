
// WPD_MTP_data.cpp : 定义应用程序的类行为。
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


// CWPD_MTP_dataApp 构造

CWPD_MTP_dataApp::CWPD_MTP_dataApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CWPD_MTP_dataApp 对象

CWPD_MTP_dataApp theApp;


// CWPD_MTP_dataApp 初始化

BOOL CWPD_MTP_dataApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	//1.日志初始化
	if (!CLogRecord::InitLogRecord())
	{
		AfxMessageBox(_T("日志初始化失败,error:创建不了[logfiles]文件夹!"));
		return FALSE;
	}
	CreateEvent(nullptr, 0, 0, _T("4f562239-2d0e-4bb0-b0ac-ad1fee1be174"));
	int err = GetLastError();
	if (183 == err)
	{
		CLogRecord::WriteRecordToFile(_T("软件重复启动,失败!"));
#ifndef DEBUG 
		return FALSE;
#endif
	}
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	if (!SUCCEEDED(hr))
	{
		AfxMessageBox(_T("COM初始化失败"));
		return FALSE;
	}

	//用户身份验证
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
		CLogRecord::WriteRecordToFile(_T("身份验证失败,用户未注册!"));
		AfxMessageBox(_T("身份验证失败,用户未注册!"));
		return FALSE;
	}
	CLogDlg logDlg;
	 ret = logDlg.DoModal();

	if (ret != IDOK)
	{
		CLogRecord::WriteRecordToFile(_T("取消登录!"));
		AfxMessageBox(_T("用户登录失败,取消登录!"));
		return FALSE;
	}
	strUserArea = logDlg.GetUserAreas();
	strUserName = logDlg.GetUserRealName();
#endif
	

	CLogRecord::WriteRecordToFile(_T("--------------程序启动1.0.5.0!--------------"));
	CWPD_MTP_dataDlg dlg;
	dlg.SetUserAreaRealName(strUserArea,strUserName);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}
	//释放COM
	CoUninitialize();

	CLogRecord::WriteRecordToFile(_T("--------------程序结束!--------------"));

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

