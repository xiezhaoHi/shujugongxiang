
// WPD_MTP_dataDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"



// CWPD_MTP_dataDlg �Ի���
class CWPD_MTP_dataDlg : public CDialogEx
{
// ����
public:
	CWPD_MTP_dataDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WPD_MTP_DATA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnMyDeviceChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtTophone();
	afx_msg void OnCbnSelchangeComboxDevices();
	afx_msg void OnInputDeviceChange(unsigned short nState, HANDLE hDevice);
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnLbnDblclkListMsg();
	afx_msg void OnTvnSelchangedTreeAreas(NMHDR *pNMHDR, LRESULT *pResult);
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	CTreeCtrl m_tree_areas; //����ѡ����
	CStatic m_static_current_area; //��ǰѡ������
	CListBox m_listShow;
	BOOL	m_threadFlag; //�̱߳�־ 0 �߳�û����,  1�߳�����
	BOOL	m_showDevicesFlag; //ˢ�� ���ڵ��豸��
	CStringArray m_strDevicesID; //�����豸��ID
	CWinThread* m_threadShowDevs; //ˢ���豸�б���߳�
	CComboBox m_combox_devices;  //ѡ���豸
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapArea; //����ѡ��
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaParent; //���� ��Ӧ�� ����ID
	CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToID; //���ؼ���Ӧ�Ľڵ�ID
	CComboBox m_combox_chooseArea;
	HTREEITEM	m_treeCtrl_curItem; //���ؼ����µ�ѡ����
	//����
private:
	CString m_strIni; //�����ļ���·��
	MySQLConInfo  m_mysqlLogin; //mysql ���ݿ��½ ��Ϣ
	CStringArray m_aryFileName;//phone �� �ļ���·��  �ָ�ɵ����ļ���
	CString m_fileName; //sqlite �ļ���
	
	CList<CStringA> m_updateMysqlData; //����mysql���ݿ� ������µ�sql���.
	CList<CStringA>	m_areasDeviceID; // ������ѡ�����м��豸ID
	//����
public:
	//�����ļ���ʼ��
	bool InitConfig();
	//��־���
	void ShowLog(CString const&);
	//�ȴ��豸 ����,���Ӻ� ��ȡָ��·���µ��ļ�,�����ݵ�pc��
	BOOL  BeginPhoneToPc(IPortableDevice* &);

	//�ɹ���pc��mysql���ݿ� �������� д���м�sqlite���ݿ�, ���Ƹ����ݿ⵽ phone��
	BOOL  BeginPcToPhone(IPortableDevice* &);

	//��ȡsqlite�е����� ����mysql ���ݿ�  ��ȡmysql ���ݿ������ ����sqlite
	BOOL  BeginSwitchData(CString const& strPath);


	//////////////////////////////////////////////////////////////////////////
	//�ֱ���
	
	//������ ��ʼ��
	void FindChildTree(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent);

	//��ʼ����������豸����
	BOOL Init_area_devices(CString const& strAreaID);

	//����mysql���ݿ�
	BOOL UpdateMysqlDB(CList<CStringA> const& list);

	//��Ҫ��ʼ ����ѡ��combox
	BOOL Synchrodata_areas();
	
	//device_type ����
	BOOL  Synchrodata_device_type(CString const&  strAreaID, CString const& strDBPath);

	//device_info ����
	BOOL  Synchrodata_device_info(CString const&  strAreaID, CString const& strDBPath);

	//work_type  ����
	BOOL  Synchrodata_work_type(CString const&  strAreaID, CString const& strDBPath);

	//work_template  ����
	BOOL  Synchrodata_work_template(CString const&  strAreaID, CString const& strDBPath);

	//work_task  ����
	BOOL Synchrodata_work_task(CString const&  strAreaID, CString const& strDBPath);

	//work_record  ����
	BOOL  Synchrodata_work_record(CString const&  strAreaID, CString const& strDBPath);

	//sys_user ����
	BOOL  Synchrodata_sys_user(CString const&  strAreaID, CString const& strDBPath);
	//////////////////////////////////////////////////////////////////////////

	afx_msg void OnBnClickedRefreshDevs();
};
