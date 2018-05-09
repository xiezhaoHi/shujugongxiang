
// startExeDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "startExeDlg.h"
#include "afxdialogex.h"
#include "verity/MyVerify.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���


// CstartExeDlg �Ի���



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


// CstartExeDlg ��Ϣ�������

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CstartExeDlg::OnPaint()
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
HCURSOR CstartExeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CstartExeDlg::OnBnClickedButtonStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strInput;
	m_eidt_input.GetWindowText(strInput);
	strInput.Trim();
	if (strInput.IsEmpty())
	{
		MessageBox(_T("������ע����!"));
		return;
	}
	//
	if (MyVerify::GetInstance()->WriteAndVerityExe(CpublicFun::UnicodeToAsc(strInput)))
	{
		//У��ɹ� ��������
		if (MyVerify::GetInstance()->VerityExe())
		{
			CDialogEx::OnOK();
		}
	}
	else
		MessageBox(_T("��������ȷ��ע����!"));
	
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnClose();
}


void CstartExeDlg::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���

	CDialogEx::OnCancel();
}


BOOL CstartExeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetWindowTextA(m_edit_show, MyVerify::GetInstance()->ReturnPcMac());
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
