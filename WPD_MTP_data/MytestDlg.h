#pragma once
#include "afxwin.h"


// MytestDlg �Ի���

class MytestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MytestDlg)

public:
	MytestDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~MytestDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit m_edit_test;
	virtual BOOL OnInitDialog();
};
