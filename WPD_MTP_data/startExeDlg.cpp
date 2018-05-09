
// startExeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "startExeDlg.h"
#include "afxdialogex.h"
#include "verity/MyVerify.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框


// CstartExeDlg 对话框



CstartExeDlg::CstartExeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_STARTEXE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CstartExeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_eidt_input);
	DDX_Control(pDX, IDC_EDIT_SHOW, m_edit_show);
}

BEGIN_MESSAGE_MAP(CstartExeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CstartExeDlg::OnBnClickedButtonStart)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CstartExeDlg 消息处理程序

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CstartExeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CstartExeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CstartExeDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strInput;
	m_eidt_input.GetWindowText(strInput);
	strInput.Trim();
	if (strInput.IsEmpty())
	{
		MessageBox(_T("请输入注册码!"));
		return;
	}
	//
	if (MyVerify::GetInstance()->WriteAndVerityExe(CpublicFun::UnicodeToAsc(strInput)))
	{
		//校验成功 启动程序
		if (MyVerify::GetInstance()->VerityExe())
		{
			CDialogEx::OnOK();
		}
	}
	else
		MessageBox(_T("请输入正确的注册码!"));
	
}


BOOL CstartExeDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		OnBnClickedButtonStart();
		return TRUE;
	}
	if (pMsg->message == WM_CLOSE )
	{
		exit(0);
		return TRUE;
	}
	else
		return CDialog::PreTranslateMessage(pMsg);
}


void CstartExeDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}


void CstartExeDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnCancel();
}


BOOL CstartExeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetWindowTextA(m_edit_show, MyVerify::GetInstance()->ReturnPcMac());
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
