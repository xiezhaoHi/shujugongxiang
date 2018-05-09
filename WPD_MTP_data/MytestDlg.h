#pragma once
#include "afxwin.h"


// MytestDlg 对话框

class MytestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MytestDlg)

public:
	MytestDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~MytestDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit m_edit_test;
	virtual BOOL OnInitDialog();
};
