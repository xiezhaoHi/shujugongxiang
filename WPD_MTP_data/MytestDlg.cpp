// MytestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "MytestDlg.h"
#include "afxdialogex.h"


// MytestDlg 对话框

IMPLEMENT_DYNAMIC(MytestDlg, CDialogEx)

MytestDlg::MytestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	

}

MytestDlg::~MytestDlg()
{
}

void MytestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TEST, m_edit_test);
}


BEGIN_MESSAGE_MAP(MytestDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &MytestDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// MytestDlg 消息处理程序


void MytestDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


BOOL MytestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
