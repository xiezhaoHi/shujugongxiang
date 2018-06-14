#pragma once
#include "afxwin.h"


// CLogDlg 对话框

//用户信息结构 --数据库信息
typedef struct user_infoStruct
{
	CString m_strName; //用户名
	CString m_strPwd; //密码
	CString m_strAreas; //用户区域
	CString m_strPwk; //加密码
	CString m_strRealName; //显示名字
}user_infoStruct;

//历史用户信息--本地记录
typedef struct user_infoRecord
{
	CString m_strName;
	CString m_strPwd;
};

class CLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLogDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	
private:
	CMap<CString, LPCTSTR, CString, LPCTSTR>	m_userMappwd;
	user_infoStruct* m_userInfo;
	int m_userInfoNum; //用户条数

	//历史用户 登陆信息
	user_infoRecord* m_userInfoRecord;
	int m_userInfoRecordNum;
	CString m_strAreas; //用户区域
	CString m_strUser; //用户名

public:
	//获取用户管理的区域
	CString GetUserAreas();
	CString GetUserRealName();

	virtual BOOL OnInitDialog();
	BOOL RecordUser(CString const& strName, CString const& strPwd);
	bool InitConfig(MySQLConInfo &);
	CComboBox m_combox_user;
	afx_msg void OnBnClickedButtonLogin();
	CEdit m_edit_pwd;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnSelchangeComboUser();
};
