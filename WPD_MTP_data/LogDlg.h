#pragma once
#include "afxwin.h"


// CLogDlg �Ի���

//�û���Ϣ�ṹ --���ݿ���Ϣ
typedef struct user_infoStruct
{
	CString m_strName; //�û���
	CString m_strPwd; //����
	CString m_strAreas; //�û�����
	CString m_strPwk; //������
	CString m_strRealName; //��ʾ����
}user_infoStruct;

//��ʷ�û���Ϣ--���ؼ�¼
typedef struct user_infoRecord
{
	CString m_strName;
	CString m_strPwd;
};

class CLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLogDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	
private:
	CMap<CString, LPCTSTR, CString, LPCTSTR>	m_userMappwd;
	user_infoStruct* m_userInfo;
	int m_userInfoNum; //�û�����

	//��ʷ�û� ��½��Ϣ
	user_infoRecord* m_userInfoRecord;
	int m_userInfoRecordNum;
	CString m_strAreas; //�û�����
	CString m_strUser; //�û���

public:
	//��ȡ�û����������
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
