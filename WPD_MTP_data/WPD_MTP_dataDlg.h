
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

	/************************************************************************/
	//���Ϳؼ� ͼƬ��Դ
	HICON		m_treeIconList[tree_max];
	CImageList	m_treeImageList;
	
	CTreeCtrl m_tree_areas; //����ѡ����
	CStatic m_static_current_area; //��ǰѡ������
	CListBox m_listShow;
	BOOL	m_threadFlag; //�̱߳�־ 0 �߳�û����,  1�߳�����
	BOOL	m_showDevicesFlag; //ˢ�� ���ڵ��豸��
	CStringArray m_strDevicesID; //�����豸��ID
	CWinThread* m_threadShowDevs; //ˢ���豸�б���߳�
	CComboBox m_combox_devices;  //ѡ���豸
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapArea; //����ѡ�� ID-����
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaParent; //���� ��Ӧ�� ID-����ID
	CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToID; //���ؼ���Ӧ�Ľڵ�ID
	CMap< CString, LPCTSTR, HTREEITEM, HTREEITEM> m_mapTreeIDToCtrl; //���ؼ��� ����ID��Ӧ�� �ؼ�item;
	//////
	//work_type ���� areas �� չ�������οؼ���
	CTreeCtrl m_tree_type;
	CMap< CString, LPCTSTR, HTREEITEM, HTREEITEM> m_mapAreaToCtr; //ϵͳ���������ؼ��ڵ�
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeToArea; //ϵͳѡ��
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapType; //ϵͳѡ�� ID-����
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParent; //�豸ID��Ӧ�ĸ�ID 
	CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToIDType; //���ؼ���Ӧ�Ľڵ�ID
	
	CComboBox m_combox_chooseArea;
	HTREEITEM	m_treeCtrl_curItem; //�������ؼ����µ�ѡ����
	HTREEITEM	m_treeCtrl_curItemType; //ϵͳwork_type���ؼ����µ�ѡ����
	//����

	//20180607
	//��ѡ��ѡ����
	int  m_radioChoose;

	//ͬ������
	int m_tongbu_dir; //��pc���ն�tongbu_to_phone = 1, ���ն˵�pc tongbu_to_pc
		

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
	//��ʼ��ͼƬ��Դ
	bool InitResource();
	//��־���
	void ShowLog(CString const&);
	//�ȴ��豸 ����,���Ӻ� ��ȡָ��·���µ��ļ�,�����ݵ�pc��
	BOOL  BeginPhoneToPc(IPortableDevice* &);

	//�ɹ���pc��mysql���ݿ� �������� д���м�sqlite���ݿ�, ���Ƹ����ݿ⵽ phone��
	BOOL  BeginPcToPhone(IPortableDevice* &);

	//��ȡsqlite�е����� ����mysql ���ݿ�  ��ȡmysql ���ݿ������ ����sqlite
	BOOL  BeginSwitchData(CString const& strPath);

	//work_type ������Ӧ����
	BOOL    InitDeviceTree(CString const&);
	//��ȡ��Ӧ�����ϵͳ
	BOOL  InitWorkTypeMap(CString const& , CStringArray & );
	//չ���������нڵ�
	void MyExpandTree(CTreeCtrl &treeCtrl,HTREEITEM hTreeItem);

	//////////////////////////////////////////////////////////////////////////
	//�ֱ���
	
	//�������豸����ID
	void FindChildTypeID(CStringArray & , HTREEITEM &);
	//ѡ�����������������
	void FindChildID(CStringArray & strAry, HTREEITEM & item);
	//������ ��ʼ��
	void FindChildTree(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent);
	//work_type ������ ���� �����ؼ���չʾ
	void  FindChildTreeType(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent);
	//��ʼ����������豸����
	//20180607
	/*
	strAryDeviceID �����豸
	*/
	BOOL Init_area_devices(CStringArray const& strAryAreaID, CStringArray & strAryDeviceID);
	//����mysql���ݿ�
	BOOL UpdateMysqlDB(CList<CStringA> const& list);

	//��Ҫ��ʼ ����ѡ��combox
	BOOL Synchrodata_areas();
	
	//device_type ����
	BOOL  Synchrodata_device_type(CString const&  strAreaID, CString const& strDBPath);

	//device_info ����
	/*20180607 �����ֶ� 
	*tongbuDir ͬ������:pc���ն� �ն˵�pc
	*tongbuFw  ͬ����Χ: ����   �豸
	*/

	BOOL  Synchrodata_device_info(CString const&  strAreaID, CString const& strDBPath, int tongbuDir,int tongbuFw);

	//work_type  ����
	/*20180607 �����ֶ�
	*tongbuDir ͬ������:pc���ն� �ն˵�pc
	*/
	BOOL  Synchrodata_work_type(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//work_template  ����
	/*20180607 �����ֶ�
	*tongbuDir ͬ������:pc���ն� �ն˵�pc
	*/
	BOOL  Synchrodata_work_template(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//work_task  ����
	/*20180607 �����ֶ�
	*tongbuDir ͬ������:pc���ն� �ն˵�pc
	*/
	BOOL Synchrodata_work_task(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//work_record  ����
	/*20180607 �����ֶ�
	*tongbuDir ͬ������:pc���ն� �ն˵�pc
	*/
	BOOL  Synchrodata_work_record(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//sys_user ����
	BOOL  Synchrodata_sys_user(CString const&  strAreaID, CString const& strDBPath);
	//////////////////////////////////////////////////////////////////////////

	afx_msg void OnBnClickedRefreshDevs();

	afx_msg void OnTvnSelchangedTreeType(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawTreeAreas(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawTreeType(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_static_curchoose;
	afx_msg void OnBnClickedButtonZdtopc();
	afx_msg void OnBnClickedRadioUsrs();
	afx_msg void OnBnClickedRadioDevices();
	afx_msg void OnBnClickedRadioWork();
	CButton m_button_zdtopc;
	CButton m_bt_tophone;
};
