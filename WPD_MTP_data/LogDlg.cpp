// LogDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "LogDlg.h"
#include "afxdialogex.h"


// CLogDlg �Ի���

IMPLEMENT_DYNAMIC(CLogDlg, CDialogEx)

CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_LOGIN, pParent)
{
	m_userInfo = nullptr;
	m_userInfoNum = 0; //�û�����
	m_userInfoRecord = nullptr;
	m_userInfoRecordNum = 0;
}

CLogDlg::~CLogDlg()
{
	if (m_userInfo)
	{
		delete [] m_userInfo;
		m_userInfoNum = 0;
	}
	if (m_userInfoRecord)
	{
		delete[] m_userInfoRecord;
	}
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_USER, m_combox_user);
	DDX_Control(pDX, IDC_EDIT_PWD, m_edit_pwd);
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLogDlg::OnBnClickedButtonLogin)
	ON_CBN_SELCHANGE(IDC_COMBO_USER, &CLogDlg::OnCbnSelchangeComboUser)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CLogDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()


// CLogDlg ��Ϣ�������

//�����ļ���ʼ��
bool CLogDlg::InitConfig(MySQLConInfo & m_mysqlLogin)
{
	CString strIni = CLogRecord::GetAppPath() + _T("//config//config.ini");
	CStringA strPath = CpublicFun::UnicodeToAsc(strIni);
	
	char buff[MAX_PATH] = { 0 };
	std::memset(buff, 0, sizeof(buff));

	//MyAec myaec;
	string strOut; 
	{
		GetPrivateProfileStringA(("DATABASE"), ("server"), ("127.0.0.1")
			, buff, MAX_PATH, strPath);
		m_mysqlLogin.server = buff;

		std::memset(buff, 0, sizeof(buff));
		///

		GetPrivateProfileStringA(("DATABASE"), ("username"), ("root")
			, buff, MAX_PATH, strPath);

		m_mysqlLogin.user = buff;

		std::memset(buff, 0, sizeof(buff));
		///

		GetPrivateProfileStringA(("DATABASE"), ("password"), ("root")
			, buff, MAX_PATH, strPath);

		m_mysqlLogin.password = buff;

		std::memset(buff, 0, sizeof(buff));
		//
		GetPrivateProfileStringA(("DATABASE"), ("database"), ("userDB")
			, buff, MAX_PATH, strPath);

		m_mysqlLogin.database = buff;

		std::memset(buff, 0, sizeof(buff));
		//

		GetPrivateProfileStringA(("DATABASE"), ("port"), ("3306")
			, buff, MAX_PATH, strPath);
		m_mysqlLogin.port = atoi(buff);

		std::memset(buff, 0, sizeof(buff));
		 
		//��ʼ����ʷ��Ϣ
		GetPrivateProfileStringA(("USER"), ("user_max"), ("0")
			, buff, MAX_PATH, strPath);
		
		m_userInfoRecordNum = atoi(buff);
		std::memset(buff, 0, sizeof(buff));
		if (m_userInfoRecordNum>0)
		{
			
			m_userInfoRecord = new user_infoRecord[m_userInfoRecordNum];
			CStringA strName, strPwd;
			for (int index = 0; index <m_userInfoRecordNum;++index)
			{
				strName.Format("name_%d", index);
				strPwd.Format("pwd_%d", index);
				GetPrivateProfileStringA(("USER"), strName, ("")
					, buff, MAX_PATH, strPath);
				m_userInfoRecord[index].m_strName = CpublicFun::AscToUnicode(buff);
				std::memset(buff, 0, sizeof(buff));

				GetPrivateProfileStringA(("USER"), strPwd, ("")
					, buff, MAX_PATH, strPath);
				m_userInfoRecord[index].m_strPwd = CpublicFun::AscToUnicode(buff);
				std::memset(buff, 0, sizeof(buff));
			}
		}

	}

	return true;
}

BOOL CLogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	//1.��ʼ������
	MySQLConInfo mysqlLogin;
	if (InitConfig(mysqlLogin))
	{
		//1.1 ��ʼ���û��б�
		CString strNameRecord, strPwdRecord;
		for (int index = 0; index < m_userInfoRecordNum;++index)
		{
			strNameRecord = m_userInfoRecord[index].m_strName;
			strPwdRecord = m_userInfoRecord[index].m_strPwd;
			if (!strPwdRecord.IsEmpty())
			{
				m_combox_user.AddString(strNameRecord);
				m_combox_user.SetCurSel(index);
				m_edit_pwd.SetWindowText(strPwdRecord);
				m_userMappwd[strNameRecord] = strPwdRecord;
			}
		}
		


		//2.��ȡ������û�ע����Ϣ
		//����mysql���ݿ�
		std::vector<std::vector<std::string> > vecDataType;
		if (CMyDataBase::GetInstance()->InitMyDataBase(mysqlLogin))
		{
			CStringA strSql;
			strSql.Format("SELECT `user_name`,\
					`password`,\
					`data_areas`,\
					`pwd_key`,real_name\
					FROM `sys_user` where  SynchronState='0' or  SynchronState='1';");
			if (CMyDataBase::GetInstance()->Select(strSql.GetBuffer(), vecDataType))
			{
				m_userInfoNum = vecDataType.size();
				if (m_userInfoNum > 0) //������Ч
				{
					int index = 0;
					CString strName;
					m_userInfo = new user_infoStruct[m_userInfoNum];
					for each (std::vector<std::string> varVec in vecDataType)
					{
						m_userInfo[index].m_strAreas = 
							CpublicFun::AscToUnicode(varVec[user_infoData_area].c_str());

						m_userInfo[index].m_strName =
							CpublicFun::AscToUnicode(varVec[user_infoData_name].c_str());

						m_userInfo[index].m_strPwd =
							CpublicFun::AscToUnicode(varVec[user_infoData_pwd].c_str());
						
						m_userInfo[index].m_strRealName =
							CpublicFun::AscToUnicode(varVec[user_infoData_realName].c_str());

						m_userInfo[index++].m_strPwk =
							CpublicFun::AscToUnicode(varVec[user_infoData_pwk].c_str());

					}
				}
			
			}
			else
			{
				CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err + strSql));
				MessageBox(_T("��ѯ���ݿ�ʧ��,���˳�����!"), _T("��ʾ"), MB_OK| MB_TOPMOST);
				SetWindowText(_T("�û���¼-�������ݿ�ʧ��!"));
		
			}
			//�ر����ݿ�
			CMyDataBase::GetInstance()->Close();

		}
		else
		{
			CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err + CStringA(":�����ݿ�ʧ��")));
			::MessageBox(nullptr,_T("�����ݿ�ʧ��,���˳�����!"),_T("��ʾ"),MB_OK|MB_TOPMOST);
			SetWindowText(_T("�û���¼-�������ݿ�ʧ��!"));
			return TRUE;
		}
		
	}
	else
	{
		CLogRecord::WriteRecordToFile(_T(":��ʼ������ʧ��"));
		MessageBox(_T("��ʼ������ʧ��,���˳�����!"), _T("��ʾ"), MB_OK | MB_TOPMOST);
		SetWindowText(_T("�û���¼-���ó�ʼ��ʧ��!"));
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

BOOL CLogDlg::RecordUser(CString const& strName, CString const& strPwd)
{
	//�ظ����û�����¼
	//1.�û����ظ�,�޸�����
	
	int curIndex = m_userInfoRecordNum;
	for (int index = 0; index < m_userInfoRecordNum; ++index)
	{
		if (m_userInfoRecord[index].m_strName == strName)
		{
			curIndex = index;
		}
	}
	CString strIni = CLogRecord::GetAppPath() + _T("//config//config.ini");
	CString strNameTemp, strPwdTemp,strUserMax;
	strNameTemp.Format(_T("name_%d"), curIndex);
	strPwdTemp.Format(_T("pwd_%d"), curIndex);
	strUserMax.Format(_T("%d"), ++curIndex);
	::WritePrivateProfileString(_T("USER"), strNameTemp, strName, strIni);
	::WritePrivateProfileString(_T("USER"), strPwdTemp, strPwd, strIni);
	::WritePrivateProfileString(_T("USER"), _T("user_max"), strUserMax, strIni);
	return TRUE;
}
//��ȡ�û����������
CString CLogDlg::GetUserAreas()
{
	return m_strAreas;
}

//��ȡ�û�������չʾ
CString CLogDlg::GetUserRealName()
{
	return m_strUser;
}
void CLogDlg::OnBnClickedButtonLogin()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strName, strPwd;
	m_combox_user.GetWindowText(strName);
	m_edit_pwd.GetWindowText(strPwd);
	if (strName.IsEmpty() || strPwd.IsEmpty())
	{
		MessageBox(_T("�����������û���,������"), _T("��ʾ"), MB_OK | MB_TOPMOST);
		return;
	}
	for (int index = 0; index < m_userInfoNum; ++index)
	{
		//������Ч
		if (m_userInfo[index].m_strName == strName)
		{
			CString strPwdTemp = strPwd + m_userInfo[index].m_strPwk;
			CStringA strPwdA = MyVerify::GetInstance()->EncryptStrMD5(CpublicFun::UnicodeToAsc(strPwdTemp));
			if (m_userInfo[index].m_strPwd == CpublicFun::AscToUnicode(strPwdA))//У��ͨ��
			{
			}
			else
			{
				m_edit_pwd.SetWindowText(_T(""));
				MessageBox(_T("�������"), _T("��ʾ"), MB_OK | MB_TOPMOST);
				return;
			}
			m_strAreas = m_userInfo[index].m_strAreas;
			m_strUser = m_userInfo[index].m_strRealName;
			//��½�ɹ�
			RecordUser(strName, strPwd);
			CDialogEx::OnOK();
			return;
		}
	}
	MessageBox(_T("δ�ҵ��û���Ϣ"), _T("��ʾ"),  MB_OK | MB_TOPMOST);
}


BOOL CLogDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		OnBnClickedButtonLogin();
		return TRUE;
	}
	if (pMsg->message == WM_CLOSE)
	{
		exit(0);
		return TRUE;
	}
	else
		return CDialog::PreTranslateMessage(pMsg);
}


void CLogDlg::OnCbnSelchangeComboUser()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strName;
	m_combox_user.GetWindowText(strName);
	m_edit_pwd.SetWindowText(m_userMappwd[strName]);
}


void CLogDlg::OnBnClickedButtonClear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strName;
	m_combox_user.GetWindowText(strName);
	if (strName.IsEmpty())
	{
		return;
	}
	int n = m_combox_user.GetCurSel();
	m_combox_user.DeleteString(n);
	m_edit_pwd.SetWindowText(_T(""));
	if (n>=0 && n < m_userInfoRecordNum)
	{
		//1.���� ����Ϊ��
		m_userInfoRecord[n].m_strPwd = _T("");
		//2.������Ϊ�� ��ʶ �û���ɾ��
		RecordUser(strName, _T(""));
	}
	if (n + 1 < m_userInfoRecordNum)
	{
		m_combox_user.SetCurSel(n + 1);
	}
	else if (n - 1 >= 0)
	{
		m_combox_user.SetCurSel(n - 1);
	}
	else
		m_combox_user.SetCurSel(-1);
}
