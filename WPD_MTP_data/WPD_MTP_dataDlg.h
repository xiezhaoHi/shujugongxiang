
// WPD_MTP_dataDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxdtctl.h"

//ɾ�����ṹ
typedef struct deleteTree
{
	CString m_strID; //��������豸��ID
	CString m_parentID;//��������豸����ID
	CString m_strName; //�豸������������
	int  m_typeFlag; //��������豸�ı�־0���� 1�豸
	BOOL  m_deleteFlag; //ɾ����־  TRUE ɾ�� FALSE��ɾ��
	struct deleteTree *m_fistchild;//��һ������
	struct deleteTree *m_nextsibling;//��һ���ֵ�
	deleteTree()
	{
		m_parentID = _T("0");
		m_strID = _T("0");
		m_strName = _T("");
		m_typeFlag = -1; //���ڵ�
		m_deleteFlag = FALSE;
		m_fistchild = nullptr;
		m_nextsibling = nullptr;
	}
}deleteTree;
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
		
					  //20180608 �м䱸�����ݿ� ����ɾ������ �ع�
	CString m_copyName;

	//20180612 ����һ��ɾ����������豸���ṹ
	deleteTree*	  m_deleteTree;
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaParentDelete; //���� ��Ӧ�� ID-����ID
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaStatusDelete; //���� ��Ӧ�� ID-ͬ��״̬  3 ��ʾɾ��
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaNameDelete; //���� ��Ӧ�� ID-����
	CMap<CString, LPCTSTR, deleteTree*, deleteTree*> m_mapAreaIDToNode; //����ID��Ӧ�Ľڵ�ID

	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeToAreaDelete; //ϵͳѡ��ID-����ID
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParentDelete; //�豸ID��Ӧ�ĸ�ID 
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeStatusDelete; //�豸ID��Ӧ��-ͬ��״̬  3 ��ʾɾ��
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeNameDelete; //�豸ID��Ӧ��-����
private:
	CString m_strIni; //�����ļ���·��
	MySQLConInfo  m_mysqlLogin; //mysql ���ݿ��½ ��Ϣ
	CStringArray m_aryFileName;//phone �� �ļ���·��  �ָ�ɵ����ļ���
	CString m_fileName; //sqlite �ļ���
	
	BOOL	m_clearFlag; //ɾ��������ʶ
	CString m_clearDeviceID; //ɾ���������豸ID

	
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

	//20180608
	//����һ��Ϊɾ������,���Խ��лع�����
	BOOL  BeginRebackData(CString const& );


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

	//20180608
	//ɾ����ʷ���� ����ѡ���ʱ�䷶Χ ɾ����Ӧ�� ��¼����ʷ
	BOOL  Synchrodata_clear_record(CString const& strDBPath,CString const& strBeginT,CString const& strEndt);

	//////////////////////////////////////////////////////////////////////////
	//20180612 ɾ������

	//��ʼ��ɾ�������ڴ�ṹ��
	BOOL  InitDeleteTreeStruct();

	//�ݹ�ɾ�����ṹ
	BOOL  ClearDeleteTreeStruct(deleteTree* &);

	//�ݹ鴴���������ṹ
	BOOL  CreateDeleteTreeStructAreas(deleteTree* &);
	//��������
	BOOL  CreateDeleteTreeStructAreasC(deleteTree* &);
	//�ݹ鴴���豸���ṹ
	BOOL  CreateDeleteTreeStructDevices(deleteTree* &);

	//ɾ����Ӧ����������豸
	BOOL  DeleteAreaAndDevice(CString const&);
	
	//�ݹ��ȡ ��Ҫɾ����������豸 �������ǵ���������豸
	BOOL  DeleteAreaAndDeviceFindID(deleteTree* &, CStringArray&, CStringArray&);

	//�ݹ��ȡ��Ҫɾ������ӽڵ� ���ӽڵ���ֵܽڵ�
	BOOL  DeleteFindID(deleteTree* & treeNode
		, CStringArray& strAryAreas, CStringArray& strAryDevices);
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
	CDateTimeCtrl m_datetime_begin;
	CDateTimeCtrl m_datetime_end;
	afx_msg void OnBnClickedButtonBeginClear();
	afx_msg void OnBnClickedButtonReback();
	afx_msg void OnDtnDatetimechangeDatetimepickerEnd(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_bt_begin_clear;
	CButton m_bt_begin_reback;
	afx_msg void OnDtnDatetimechangeDatetimepickerBegin(NMHDR *pNMHDR, LRESULT *pResult);

	CStatic m_static_tree_show;
};
