// MytestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "MytestDlg.h"
#include "afxdialogex.h"


// MytestDlg �Ի���

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


// MytestDlg ��Ϣ�������


void MytestDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
}


BOOL MytestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
