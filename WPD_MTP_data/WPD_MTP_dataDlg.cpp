
// WPD_MTP_dataDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WPD_MTP_data.h"
#include "WPD_MTP_dataDlg.h"
#include "afxdialogex.h"
#include <Dbt.h>
#include <map>
//#include "MyAec.h"
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWPD_MTP_dataDlg �Ի���
// Device enumeration
DWORD EnumerateAllDevices();
void ChooseDevice(_Outptr_result_maybenull_ IPortableDevice** device);
BOOL ChooseDevice(
	_Outptr_result_maybenull_ IPortableDevice** device, CString strDevID, PWSTR buffErr);
//���� ��ȡ���е��豸������  bufferr���������Ϣ wchar buffErr[1024] = {0};
BOOL GetWDPAllDevice(CStringArray & strArr, CStringArray& strArrName, PWSTR buffErr);
void GetClientInformation(
	_Outptr_result_maybenull_ IPortableDeviceValues** clientInformation);
HRESULT OpenDevice(LPCWSTR wszPnPDeviceID, IPortableDevice** ppDevice);


// Content enumeration
void EnumerateAllContent(_In_ IPortableDevice* device, _In_ PCWSTR fileName);
void ReadHintLocations(_In_ IPortableDevice* device);

// Content transfer
void TransferContentFromDevice(_In_ IPortableDevice* device);
void TransferContentToDevice(
	_In_ IPortableDevice* device,
	_In_ REFGUID          contentType,
	_In_ PCWSTR           fileTypeFilter,
	_In_ PCWSTR           defaultFileExtension);
void TransferContactToDevice(_In_ IPortableDevice* device);
void CreateFolderOnDevice(_In_ IPortableDevice* device);
void CreateContactPhotoResourceOnDevice(_In_ IPortableDevice* device);

// Content deletion
void DeleteContentFromDevice(_In_ IPortableDevice* device);

// Content moving
void MoveContentAlreadyOnDevice(_In_ IPortableDevice* device);

// Content update (properties and data simultaneously)
void UpdateContentOnDevice(
	_In_ IPortableDevice* device,
	_In_ REFGUID          contentType,
	_In_ PCWSTR           fileTypeFilter,
	_In_ PCWSTR           defaultFileExtension,
	_In_	PCWSTR strID);

// Content properties
void ReadContentProperties(_In_ IPortableDevice* device);
void WriteContentProperties(_In_ IPortableDevice* device);
void ReadContentPropertiesBulk(_In_ IPortableDevice* device);
void WriteContentPropertiesBulk(_In_ IPortableDevice* device);
void ReadContentPropertiesBulkFilteringByFormat(_In_ IPortableDevice* device);

// Functional objects
void ListFunctionalObjects(_In_ IPortableDevice* device);
void ListFunctionalCategories(_In_ IPortableDevice* device);
void ListSupportedContentTypes(_In_ IPortableDevice* device);
void ListRenderingCapabilityInformation(_In_ IPortableDevice* device);

// Device events
void ListSupportedEvents(_In_ IPortableDevice* device);
void RegisterForEventNotifications(_In_ IPortableDevice* device, _Inout_ PWSTR* eventCookie);
void UnregisterForEventNotifications(_In_opt_ IPortableDevice* device, _In_opt_ PCWSTR eventCookie);

// Misc.
void GetObjectIdentifierFromPersistentUniqueIdentifier(_In_ IPortableDevice* device);

//��ȡShouChiZhongDuan�ļ���ID
PCWSTR getSpecifiedObjectID(_In_ IPortableDevice* device, PCWSTR fileName);
void TransforDataFromDevice(_In_ IPortableDevice* device, _In_ PCWSTR objectID);
map<wstring, wstring> childIDs;    //�����Ҫ���ݵ��ļ��Ķ���ID<->����
void getChildIDs(_In_ IPortableDevice* device, _In_ IPortableDeviceContent* content, _In_ PCWSTR parentID);
PCWSTR getIDByParentID(_In_ IPortableDevice* device, _In_ IPortableDeviceContent* content, _In_ PCWSTR parentID, _In_ PCWSTR name);
BOOL  TransferDataToDevice(
	_In_ IPortableDevice* device,
	_In_ REFGUID          contentType,
	_In_ PCWSTR parentID
	, PCWSTR strFilePath
	, PWSTR strErr);
void DeleteDataFromDevice(_In_ IPortableDevice* device, _In_ PCWSTR objectID);

void DoMenu()
{
	HRESULT hr = S_OK;
	UINT    selectionIndex = 0;
	PWSTR   eventCookie = nullptr;
	WCHAR   selectionString[SELECTION_BUFFER_SIZE] = { 0 };
	ComPtr<IPortableDevice> device;

	ChooseDevice(&device);  //Ĭ�ϵ�һ���豸

	if (device == nullptr)
	{
		// No device was selected, so exit immediately.
		return;
	}

	//��ȡShouChiZhongDuan�ļ��е�ID
	PCWSTR objID = getSpecifiedObjectID(device.Get(), L"����");  //koudaigouwu.apk
															   //��ȡShouChiZhongDuan/data��ShouChiZhongDuan/data/phone.db��ID
	ComPtr<IPortableDeviceContent>  content;
	hr = device->Content(&content);
	if (FAILED(hr))
	{
		wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
		return;
	}
	PCWSTR dataID = getIDByParentID(device.Get(), content.Get(), objID, L"data");
	PCWSTR phoneID = getIDByParentID(device.Get(), content.Get(), dataID, L"phone");
	//ɾ���ֳ��豸�ϵ�phone.db
	DeleteDataFromDevice(device.Get(), phoneID);
	//����phone.db���ֳ��豸��
	//TransferDataToDevice(device.Get(), WPD_CONTENT_TYPE_ALL, dataID,gFilePath);



	CoTaskMemFree(eventCookie);
}


/************************************************************************/
//�������


CString	gDirID, gFileID; //phone�� sqlite ���ݿ���λ��ID  �� �ļ���ID
CString gFilePath; //�м��ļ��� ·��
/************************************************************************/

CWPD_MTP_dataDlg::CWPD_MTP_dataDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_WPD_MTP_DATA_DIALOG, pParent)
{
	m_showDevicesFlag = TRUE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWPD_MTP_dataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MSG, m_listShow);
	DDX_Control(pDX, IDC_COMBOX_Devices, m_combox_devices);
	DDX_Control(pDX, IDC_COMBOX_QyChoose, m_combox_chooseArea);
	DDX_Control(pDX, IDC_TREE_AREAS, m_tree_areas);
	DDX_Control(pDX, IDC_STATIC_Area, m_static_current_area);
}

BEGIN_MESSAGE_MAP(CWPD_MTP_dataDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_TOPHONE, &CWPD_MTP_dataDlg::OnBnClickedBtTophone)
	ON_CBN_SELCHANGE(IDC_COMBOX_Devices, &CWPD_MTP_dataDlg::OnCbnSelchangeComboxDevices)
	ON_WM_INPUT_DEVICE_CHANGE()
	ON_MESSAGE(WM_DEVICECHANGE, OnMyDeviceChange)
	ON_WM_CLOSE()
	ON_LBN_DBLCLK(IDC_LIST_MSG, &CWPD_MTP_dataDlg::OnLbnDblclkListMsg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_AREAS, &CWPD_MTP_dataDlg::OnTvnSelchangedTreeAreas)
	ON_BN_CLICKED(IDC_REFRESH_DEVS, &CWPD_MTP_dataDlg::OnBnClickedRefreshDevs)
END_MESSAGE_MAP()


// CWPD_MTP_dataDlg ��Ϣ�������
//����ˢ�� �������ӵ��豸
UINT ShowAllDevices(LPVOID pParam)
{
	CWPD_MTP_dataDlg* pDlg = (CWPD_MTP_dataDlg*)pParam;
	
	WCHAR buffErr[1024] = { 0 };
	CStringArray strNames;
	while (pDlg->m_showDevicesFlag)
	{
		pDlg->m_strDevicesID.RemoveAll();
		strNames.RemoveAll();
		if (!GetWDPAllDevice(pDlg->m_strDevicesID, strNames, buffErr))
			CLogRecord::WriteRecordToFile(buffErr);
		else
		{
			pDlg->m_combox_devices.ResetContent();
			for (int index = 0; index < pDlg->m_strDevicesID.GetSize(); ++index)
			{
				pDlg->m_combox_devices.AddString(strNames.GetAt(index));
				pDlg->m_combox_devices.SetCurSel(index);
			}
		}
		//pDlg->ShowLog(_T("ɨ���豸��Ϣ!"));
		ASSERT(pDlg->m_threadShowDevs);
		SuspendThread(pDlg->m_threadShowDevs->m_hThread);
	}
	return 0;
}

static const GUID GUID_DEVINTERFACE_LIST[] = { 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
BOOL CWPD_MTP_dataDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	/************************************************************************/
	//����wm_devicechange��Ϣ ע�� ��ע�� ������Ӧ

	CLogRecord::WriteRecordToFile(_T("��ʼ��ʼ������!"));

	HDEVNOTIFY hDevNotify;
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	std::memset(&NotificationFilter, 0, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	for (int i = 0; i < sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++)
	{
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
		hDevNotify = RegisterDeviceNotification(this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if (!hDevNotify)
		{
			AfxMessageBox(CString("Can't register device notification: ")
				+ _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	
	/************************************************************************/
	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//1.��ʼ�� ����
	if (!InitConfig())
	{
		ShowLog(_T("�����ļ���ʼ��ʧ��!"));
	}
	else
		ShowLog(_T("�����ļ���ʼ���ɹ�!"));

	//��ʼ������combox
	Synchrodata_areas();


	//���ȼ�Ϊ��һ�㡱���̣߳�Ĭ��ջ��С������ʱ���� CREATE_SUSPENDED
	m_threadShowDevs = AfxBeginThread(ShowAllDevices, this,THREAD_PRIORITY_NORMAL, 0);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CWPD_MTP_dataDlg::OnPaint()
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
HCURSOR CWPD_MTP_dataDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void MyStringSplit(CString source, CStringArray& dest, TCHAR division)
{
	if (source.IsEmpty())
	{

	}
	else
	{
		int pos = source.Find(division);
		if (pos == -1)
		{
			dest.Add(source);
		}
		else
		{
			dest.Add(source.Left(pos));
			source = source.Mid(pos + 1);
			MyStringSplit(source, dest, division);
		}
	}
}
//�����ļ���ʼ��
bool CWPD_MTP_dataDlg::InitConfig()
{
	CString strIni = CLogRecord::GetAppPath() + _T("//config//config.ini");
	
	string strAecKey = "0123456789ABCDEF0123456789ABCDEF";//256bits, also can be 128 bits or 192bits  
	string aesIV = "ABCDEF0123456789";//128 bits
	
	CStringA strPath = CpublicFun::UnicodeToAsc(strIni);
	m_strIni = strIni;
	char buff[MAX_PATH] = { 0 };

	CStringA aecFlag; //aec ���ܱ�ʶ  0δ����  1����

	GetPrivateProfileStringA(("DATABASE"), ("aec"), "0"
		, buff, MAX_PATH, strPath);
	aecFlag = buff;
	std::memset(buff, 0, sizeof(buff));

	//MyAec myaec;
	string strOut; //�м���� ���� ���ܷ��ص�����
// 	if (aecFlag == "1") //�Ѿ����ܹ���
// 	{
// 		GetPrivateProfileStringA(("DATABASE"), ("server"), ("127.0.0.1")
// 			, buff, MAX_PATH, strPath);
// 		strOut = myaec.CBC_AESDecryptStr(strAecKey, aesIV, buff);
// 		m_mysqlLogin.server = strOut.c_str();
// 		std::memset(buff, 0, sizeof(buff));
// 
// 		GetPrivateProfileStringA(("DATABASE"), ("username"), ("root")
// 			, buff, MAX_PATH, strPath);
// 		strOut = myaec.CBC_AESDecryptStr(strAecKey, aesIV, buff);
// 		m_mysqlLogin.user = strOut.c_str();
// 		std::memset(buff, 0, sizeof(buff));
// 
// 		GetPrivateProfileStringA(("DATABASE"), ("password"), ("root")
// 			, buff, MAX_PATH, strPath);
// 		strOut = myaec.CBC_AESDecryptStr(strAecKey, aesIV, buff);
// 		m_mysqlLogin.password = strOut.c_str();
// 		std::memset(buff, 0, sizeof(buff));
// 
// 		GetPrivateProfileStringA(("DATABASE"), ("database"), ("userDB")
// 			, buff, MAX_PATH, strPath);
// 		strOut = myaec.CBC_AESDecryptStr(strAecKey, aesIV, buff);
// 		m_mysqlLogin.database = strOut.c_str();
// 		std::memset(buff, 0, sizeof(buff));
// 
// 		GetPrivateProfileStringA(("DATABASE"), ("port"), ("3306")
// 			, buff, MAX_PATH, strPath);
// 		strOut = myaec.CBC_AESDecryptStr(strAecKey, aesIV, buff);
// 		m_mysqlLogin.port = atoi(strOut.c_str());
// 		std::memset(buff, 0, sizeof(buff));
// 	}
// 	else //��һ�μ���	////���� ���ݿ� ������Ϣ ����	
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
		//
		//�޸�Ϊ���� ״̬ 1
		//WritePrivateProfileStringA(("DATABASE"), ("aec"), "1", strPath);
	}
	//////////////////////////////////////////////////////////////////////////
	//�ָ��ַ���
	TCHAR bufGUID[MAX_PATH] = { 0 };

	GetPrivateProfileString(_T("PHONE"), _T("path"), _T("")
		, bufGUID, MAX_PATH, strIni);
	CString strphonePath = bufGUID;
	std::memset(bufGUID, 0, sizeof(bufGUID));

	GetPrivateProfileString(_T("PHONE"), _T("split"), _T("")
		, bufGUID, MAX_PATH, strIni);
	CString strSplite = bufGUID;
	std::memset(bufGUID, 0, sizeof(bufGUID));

	GetPrivateProfileString(_T("PHONE"), _T("type"), _T("")
		, bufGUID, MAX_PATH, strIni);
	CString strType = bufGUID;
	std::memset(bufGUID, 0, sizeof(bufGUID));


	MyStringSplit(strphonePath, m_aryFileName, *(strSplite.GetBuffer()));

	if (m_aryFileName.IsEmpty() || strType.IsEmpty())
	{
		return false;
	}

	m_fileName = m_aryFileName.GetAt(m_aryFileName.GetSize() - 1) + strType;
	return true;
}
//��־���
void CWPD_MTP_dataDlg::ShowLog(CString const& strLog)
{
	m_listShow.AddString(strLog);
	CLogRecord::WriteRecordToFile(strLog);
}

//Ѱ�� �ӽڵ� �� ���뵽���ؼ�
//����Ϊ ���ڵ� �� ��Ӧ���ؼ��� ���
void  CWPD_MTP_dataDlg::FindChildTree(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent)
{
	//1.���� �� û�и��� ��˵��ȫ�� Ҷ�ڵ� ����
	if (mapParent.GetSize() <= 0)
	{
		return;
	}
	//2.�� ��Ӧ���׵�ֱ�� �ӽڵ�
	CString strKey,strValue;
	HTREEITEM valueItem;
	CString parentKey;
	
	POSITION posParent = mapParent.GetStartPosition();

		while (posParent)
		{
			mapParent.GetNextAssoc(posParent, parentKey, valueItem);
			CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItem; //���ڵ� ��Ӧ�� ���ؼ��ڵ� ���
			POSITION pos = m_mapAreaParent.GetStartPosition();
			while (pos)
			{
				m_mapAreaParent.GetNextAssoc(pos, strKey, strValue);//���� ���е� �ڵ�				
				if (parentKey == strValue) //���ýڵ�Ϊ���ڵ�
				{
					HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey], valueItem);// �ڸ���������Parent
					mapParentItem[strKey] = item; //�����µ� ���ڵ�
					m_mapTreeCtrToID[item] = strKey;
				}
			}
			//�ݹ� ��ȡ
			FindChildTree(mapParentItem);
		}
}

//����mysql���ݿ�
BOOL CWPD_MTP_dataDlg::UpdateMysqlDB(CList<CStringA> const& list)
{

	POSITION posUpdate = list.GetHeadPosition();
	string strTemp;
	int sqlNum = 0;
	while (posUpdate != NULL)
	{
		if (++sqlNum > UPDATEMAX)
		{
			if (!CMyDataBase::GetInstance()->Query(strTemp))
			{
				return FALSE;
			}
			strTemp = ""; //�����м����
			continue;
		}
		else
		{
			strTemp += list.GetNext(posUpdate);
		}
		//pDC->TextOut(200, 200, );//�����������View���е�OnDraw()

	}
	if (strTemp != "" )
		return CMyDataBase::GetInstance()->Query(strTemp);
	else if(strTemp != "" && sqlNum == 1)
		return CMyDataBase::GetInstance()->OneQuery(strTemp);

}

//��ʼ�� ����combox
BOOL CWPD_MTP_dataDlg::Synchrodata_areas()
{
	try {


		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
														//2.��ȡ mysql���ݿ�� �����
		if (CMyDataBase::GetInstance()->InitMyDataBase(m_mysqlLogin))
		{
			if (CMyDataBase::GetInstance()->Select("SELECT `areas`.`Id`,\
			`areas`.`Code`,\
			`areas`.`Name`,\
			`areas`.`ParentId`,\
			`areas`.`CreateDate`,\
			`areas`.`CreateUserId`\
			FROM `areas`", vecData))
			{
				int index = 0;
				CString strName;
				for each (std::vector<std::string> varVec in vecData)
				{
					//����ѡ�� ����ID
					strName = CpublicFun::AscToUnicode(varVec[areas_Name].c_str());
					m_mapArea[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] = strName;
					//m_combox_chooseArea.AddString(strName);
					m_mapAreaParent[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] = CpublicFun::AscToUnicode(varVec[areas_ParentId].c_str());
				}

				//m_combox_chooseArea.SetCurSel(0);
			}
		}
		else
		{
			CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
			ShowLog(_T("mysql���ݿ�����ʧ��,��ȷ�����������������!"));
			return FALSE;
		}
		//�ر����ݿ�
		CMyDataBase::GetInstance()->Close();


		DWORD dwStyles = m_tree_areas.GetStyle();//��ȡ������ԭ���  
		dwStyles |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
		m_tree_areas.ModifyStyle(0, dwStyles);
		//��ʼ�� ���ؼ�
		//1.�Ȳ��� ���׽ڵ�
		POSITION pos = m_mapAreaParent.GetStartPosition();
		CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItem; //���ڵ� ��Ӧ�� ���ؼ��ڵ� ���
		CString strKey, strValue;
		while (pos)
		{
			m_mapAreaParent.GetNextAssoc(pos, strKey, strValue);
			if (strValue == _T("0")) //Ϊ���׽ڵ�
			{
				HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey], TVI_ROOT);// �ڸ���������Parent
				mapParentItem[strKey] = item;
				m_mapTreeCtrToID[item] = strKey;
			}
		}

		//�ݹ鹹�� ���� ��
		FindChildTree(mapParentItem);

		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}

/*ͬ��work_record������,

1.ͬ��phone�� ��¼���ݵ� pc��

��Ҫ���� ѡ������ ���豸ID ����
strAreaID  ѡ������ID ��ʱ����
��������:˫��ͬ�� 
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_record(CString const&  strAreaID, CString const& strDBPath)
{
	try {
		//��ѡ����û���豸 
// 		if (m_areasDeviceID.IsEmpty())
// 		{
// 			CLogRecord::WriteRecordToFile(_T("û��������Ҫͬ��,��[work_record]�������!"));
// 			return TRUE;
// 		}
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;


		//0.ͬ��phone�˵ļ�¼

		CList<CStringA> listUpdateSql; //������Ҫ���µ�sql���
		CSQLite sqOne;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			int RFidCount = 0,RFidNum=0;
			BOOL ret = FALSE;
			strSql.Format(_T("SELECT count(*) FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
				, strBeginTime, strCurTime);
			//char buf[MAX_PATH] = { 0 };
			if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
			//if (sqOne.QuickSelectDataCount(strSql,RFidCount))
			{
				//RFidCount = atoi(buf);
				if (RFidCount > 0)
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[RFidCount]; //ͬ����mysql���ݿ�
					CStringArray** ppAryDataSqlite = new CStringArray*[RFidCount]; //����phone��sqlite ���� Id �� ͬ��״̬
					strSql.Format(_T("SELECT `Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`Executor`,ClassName FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
						, strBeginTime, strCurTime);

					if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
					{
						//����ת��
					
						CString strRepTemp; //�м����
						for (int index = 0; index < RFidCount; ++index)
						{
							ppAryDataSqlite[index] = new CStringArray;
							ppAryDataSqlite[index]->Add(ppAryData[index]->GetAt(work_record_Id));


							//������ת��
							for (int ind = 0; ind < work_record_max;++ind)
							{
								
								strRepTemp = ppAryData[index]->GetAt(ind);
								if (-1 != strRepTemp.Find(_T("\\")))
								{
									strRepTemp.Replace(_T("\\"), _T("\\\\"));
									ppAryData[index]->SetAt(ind, strRepTemp);
								}
							}



							strTemp.Format("REPLACE INTO work_record(`Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,ClassName)\
							values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s');"
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_Id))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_DeviceId))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_WorkTaskId))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_Number))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_ProcessName))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_Results))
								, "1"//ͬ��״̬ 1��ͬ��
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_CreateDate))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_CreateUserId))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_ClassName))
							);
							listUpdateSql.AddTail(strTemp);
						}
						RFidNum = ppAryDataSqlite[0]->GetSize(); //��ȡ������
						//����phone��sqlite ״̬
						if (!sqOne.QuickInsertData("UPDATE work_record set SynchronState ='1' where Id=?;", ppAryDataSqlite, RFidCount, RFidNum, CSQLite::sqlite3_bind))
						{
							CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
							sqOne.CloseDataBase();
							return FALSE;
						}
							
						//���� ����
						for (int index = 0; index < RFidCount; ++index)
						{
							delete ppAryData[index];
						}
						for (int index = 0; index < RFidCount; ++index)
						{
							delete ppAryDataSqlite[index];
						}
					}


					delete[] ppAryData;
					delete[] ppAryDataSqlite;

				}
			}
			sqOne.CloseDataBase();
			if (!ret)
			{
				CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
				return FALSE;
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			return FALSE;
		}

		//����mysql���ݿ�
		if (!UpdateMysqlDB(listUpdateSql))
		{
			ShowLog(_T("��phone���¼�¼���ݷ����work_recordʧ��!"));
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}
		
		//ͬ������˵�����

		strSql.Format(_T("SELECT `Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,ClassName FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
			, strBeginTime, strCurTime);
		//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//�����ݿ���ͬ��
			{
				CSQLite sq;
				if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					int index = 0;

					for each (std::vector<string> varVec in vecData)
					{
						//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
						strTemp.Format(("UPDATE work_record set SynchronState ='1'  where Id='%s';")
							, (varVec[work_record_Id].c_str()));
						m_updateMysqlData.AddTail(strTemp.GetBuffer());
						ppAryData[index] = new CStringArray;
						for each (string var in varVec)
						{
							ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
						}
						ppAryData[index]->SetAt(work_record_SynchronState, _T("1"));
						++index;
					}
					dataNum = ppAryData[0]->GetSize();
					//ִ�� û�оͲ���  �о͸���
					CStringA strSql = "REPLACE INTO work_record(`Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`Executor`,ClassName) VALUES(?,?,?,?,?,?,?,?,?,?);";

					BOOL ret = sq.QuickInsertData(strSql
						, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
					for (int index = 0; index < dataCount; ++index)
					{
						delete ppAryData[index];
					}

					delete[] ppAryData;


					sq.CloseDataBase();

					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
						return FALSE;
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}

		CLogRecord::WriteRecordToFile(_T("ͬ����[work_record]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}


/*ͬ��work_task������,
1.pc����phone��ͬ��δͬ��������, ���� ͬ��״̬.
2.����ͬ��״̬Ϊ ��ɾ�� �� ɾ��phone������
3.ͬ��phone�� ���״̬��RFId

��Ҫ���� ѡ������ ���豸ID ����
strAreaID  ѡ������ID ��ʱ����
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_task(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{
		//��ѡ����û���豸 
		if (m_areasDeviceID.IsEmpty())
		{
			return TRUE;
		}
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;

		POSITION posDeviceID = m_areasDeviceID.GetHeadPosition();
		CString strDeviceID;
		int sqlNum = 0;
		while (posDeviceID != NULL)
		{
			//��ȡ����ID
			strDeviceID = CpublicFun::AscToUnicode(m_areasDeviceID.GetNext(posDeviceID));
			CStringArray** ppAryDataUpdate =nullptr;
			//0.ͬ��phone�˵� RFid �� State�ֶ�
			int RFidCount = 0;
			CList<CStringA> listUpdateSql; //������Ҫ���µ�sql���
			CSQLite sqOne;
			if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
			{
				
				BOOL ret = FALSE;
				strSql.Format(_T("SELECT count(*) FROM `work_task` where DeviceId='%s' and SynchronState='0' and  CreateDate >= '%s' and CreateDate <= '%s';")
					, strDeviceID, strBeginTime, strCurTime);
				if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
				{
					if (RFidCount > 0)
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[RFidCount];
						ppAryDataUpdate = new CStringArray*[RFidCount];
						strSql.Format(_T("SELECT `Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId` ,Level,Frequency FROM `work_task` \
					where DeviceId='%s' and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';\
					"), strDeviceID, strBeginTime, strCurTime);
						if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
						{
							
							for (int index = 0; index < RFidCount; ++index)
							{

								CStringA strTemp = ("Replace  INTO `work_task` (`Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency) VALUES(");
								for (int indEnum = work_task_Id;indEnum < work_task_max-1;++indEnum)
								{
									if (indEnum == work_task_SynchronState)
									{
										strTemp +=  ("'1',");
									}
									else
										strTemp += "'"+ CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(indEnum)) + "'"+(",");
								}
								strTemp += "'" + CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_task_max-1)) + "');";
								listUpdateSql.AddTail(strTemp);
								ppAryDataUpdate[index] = new CStringArray;
								ppAryDataUpdate[index]->Add(ppAryData[index]->GetAt(work_task_Id));
							}
							for (int index = 0; index < RFidCount; ++index)
							{
								delete ppAryData[index];
							}
						}


						delete[] ppAryData;
						
					}
				}
				sqOne.CloseDataBase();
				if (!ret)
				{
					CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
					return FALSE;
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
				return FALSE;
			}

			//����mysql���ݿ�
			if (!UpdateMysqlDB(listUpdateSql))
			{
				ShowLog(_T("��phone����RFId�����״̬�������work_taskʧ��!"));
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));

				return FALSE;
			}
			else
			{
				if (ppAryDataUpdate)
				{
					if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						//����phone ������ ״̬
						if (sqOne.QuickInsertData("update work_task set SynchronState='1' where Id = ?", ppAryDataUpdate, RFidCount, 1, CSQLite::sqlite3_bind))
						{

						}
						sqOne.CloseDataBase();
					}
					for (int index = 0; index < RFidCount; ++index)
					{
						delete ppAryDataUpdate[index];
					}
					delete[] ppAryDataUpdate;
				}
				
			}


			strSql.Format(_T("SELECT `Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency FROM `work_task` \
		where DeviceId='%s' and SynchronState='0' and State='1' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
							strTemp.Format(("UPDATE work_task set SynchronState ='1' ,State='2' where Id='%s';")
								, (varVec[work_task_Id].c_str()));
							m_updateMysqlData.AddTail(strTemp.GetBuffer());
							ppAryData[index] = new CStringArray;
							for each (string var in varVec)
							{
								ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
							}
							ppAryData[index]->SetAt(work_task_SynchronState, _T("1"));
							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						//ִ�� û�оͲ���  �о͸���
						CStringA strSql = "Replace  INTO `work_task` (`Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

						BOOL ret = sq.QuickInsertData(strSql
							, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}

						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `Id` FROM  `work_task` where SynchronState='2' and DeviceId ='%s' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_template_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sq.QuickDeletData("delete from work_task where Id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);
						if (ret)
						{
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
						}

						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

		}


		CLogRecord::WriteRecordToFile(_T("ͬ����[work_task]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}


/*ͬ��work_template������,
1.pc����phone��ͬ��δͬ��������, ���� ͬ��״̬.
2.����ͬ��״̬Ϊ ��ɾ�� �� ɾ��phone������

��Ҫ���� ѡ������ ���豸ID ����
strAreaID  ѡ������ID ��ʱ����
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_template(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{

		//��ѡ����û���豸 
		if (m_areasDeviceID.IsEmpty())
		{
			CLogRecord::WriteRecordToFile(_T("û��������Ҫͬ��,��[work_template]�������!"));
			return TRUE;
		}
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;

		POSITION posDeviceID = m_areasDeviceID.GetHeadPosition();
		CString strDeviceID;
		int sqlNum = 0;
		while (posDeviceID != NULL)
		{
			//��ȡ����ID
			strDeviceID = CpublicFun::AscToUnicode(m_areasDeviceID.GetNext(posDeviceID));

			strSql.Format(_T("SELECT te.`Id`,te.`DeviceId`,te.`WorkTypeId`,te.ClassName,te.`Number`,te.`ProcessName`,te.`Results`,te.`SynchronState`,te.`CreateDate`,te.`CreateUserId` FROM `work_template` te,work_type tk \
		where tk.DeviceId='%s'and tk.CreateDate >= '%s' and tk.CreateDate <= '%s' and tk.Id = te.WorkTypeId;\
		"), strDeviceID, strBeginTime, strCurTime);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
							strTemp.Format(("UPDATE work_template set SynchronState ='1' where Id='%s';")
								, (varVec[work_template_Id].c_str()));
							m_updateMysqlData.AddTail(strTemp.GetBuffer());
							ppAryData[index] = new CStringArray;
							for each (string var in varVec)
							{
								ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
							}
							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						//ִ�� û�оͲ���  �о͸���
						CStringA strSql = "Replace  INTO `work_template` (`Id`,`DeviceId`,`WorkTypeId`,ClassName,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`) VALUES(?,?,?,?,?,?,?,?,?,?);";

						BOOL ret = sq.QuickInsertData(strSql
							, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}
						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `Id` FROM  `work_template` where SynchronState='2' and DeviceId ='%s' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_template_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sq.QuickDeletData("delete from work_template where Id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);
						if (ret)
						{
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
						}
						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `Id` FROM  `work_template` where SynchronState='2';\
		"));
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_template_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sq.QuickDeletData("delete from work_template where Id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);
						if (ret)
						{
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
						}

						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
		}


		CLogRecord::WriteRecordToFile(_T("ͬ����[work_template]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}

}


/*ͬ��work_type������,
1.pc����phone��ͬ��δͬ��������, ���� ͬ��״̬.
2.����ͬ��״̬Ϊ ��ɾ�� �� ɾ��phone������

strAreaID  ѡ������ID ��ʱ����
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_type(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{
		//��ѡ����û���豸 
		if (m_areasDeviceID.IsEmpty())
		{
			CLogRecord::WriteRecordToFile(_T("û��������Ҫͬ��,��[work_template]�������!"));
			return TRUE;
		}

		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;
		POSITION posDeviceID = m_areasDeviceID.GetHeadPosition();
		CString strDeviceID;
		int sqlNum = 0;
		while (posDeviceID != NULL)
		{
			//��ȡ����ID
			strDeviceID = CpublicFun::AscToUnicode(m_areasDeviceID.GetNext(posDeviceID));
			strSql.Format(_T("SELECT `Id`,`Name`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,DeviceId FROM `work_type`\
		 where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId='%s';\
		"), strBeginTime, strCurTime,strDeviceID);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
							strTemp.Format(("UPDATE work_type set SynchronState ='1' where Id='%s';")
								, (varVec[work_type_Id].c_str()));
							m_updateMysqlData.AddTail(strTemp.GetBuffer());
							ppAryData[index] = new CStringArray;
							for each (string var in varVec)
							{
								ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
							}
							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						//ִ�� û�оͲ���  �о͸���
						CStringA strSql = "Replace  INTO `work_type` (`Id`,`Name`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,DeviceId) VALUES(?,?,?,?,?,?,?,?);";

						BOOL ret = sq.QuickInsertData(strSql
							, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}
						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `Id` FROM  `work_type` where SynchronState='2' and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId='%s';\
		"), strBeginTime, strCurTime,strDeviceID);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_type_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sq.QuickDeletData("delete from work_type where Id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);
						if (ret)
						{
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
						}
						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
			
		}
		CLogRecord::WriteRecordToFile(_T("ͬ����[work_type]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}


/*ͬ��device_info������,
1.phone�� ��ѯRFid ͬ����mysql ���ݿ���
2.pc����phone��ͬ��δͬ��������, ���� ͬ��״̬.
3.����ͬ��״̬Ϊ ��ɾ�� �� ɾ��phone������
3.��¼��ѯ�����豸ID �����ѯ �����ı�  �� ��Ӧ�豸������
strAreaID  ѡ������ID
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_device_info(CString const&  strAreaID,CString const& strDBPath)
{
	try
	{
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));

		//0.ͬ��phone�˵� RFid �ֶ�
		CString strSql;
		CList<CStringA> listUpdateSql; //������Ҫ���µ�sql���
		CSQLite sqOne;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			int RFidCount = 0;
			BOOL ret = FALSE;
			strSql.Format(_T("SELECT count(*) FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s';")
				, strAreaID, strBeginTime, strCurTime);
			if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
			{
				if (RFidCount > 0)
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[RFidCount];

					strSql.Format(_T("SELECT `Id`,`RFID` FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s';")
						, strAreaID, strBeginTime, strCurTime);

					if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
					{
						for (int index = 0; index < RFidCount; ++index)
						{
							strTemp.Format(("UPDATE device_info set RFID ='%s' where Id='%s';")
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(device_info_RFID))
								, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(device_info_Id)));
							listUpdateSql.AddTail(strTemp);
						}
						for (int index = 0; index < RFidCount; ++index)
						{
							delete ppAryData[index];
						}
					}


					delete[] ppAryData;

				}
			}
			sqOne.CloseDataBase();
			if (!ret)
			{
				CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
				return FALSE;
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			return FALSE;
		}

		//����mysql���ݿ�
		if (!UpdateMysqlDB(listUpdateSql))
		{
			ShowLog(_T("��phone����RFId�������ʧ��!"));
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}


		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������

		strSql.Format(_T("SELECT `Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,\
		`SynchronState`,`CreateDate`,`CreateUserId` FROM `device_info` where AreaId='%s'and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
			, strAreaID, strBeginTime, strCurTime);
		//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//�����ݿ���ͬ��
			{
				CSQLite sq;
				if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					int index = 0;

					for each (std::vector<string> varVec in vecData)
					{
						//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
						strTemp.Format(("UPDATE device_info set SynchronState ='1' where Id='%s';")
							, (varVec[device_info_Id].c_str()));
						m_updateMysqlData.AddTail(strTemp.GetBuffer());
						//m_areasDeviceID.AddTail(varVec[device_info_Id].c_str());
						ppAryData[index] = new CStringArray;
						for each (string var in varVec)
						{
							ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
						}
						ppAryData[index]->SetAt(device_info_SynchronState, _T("1"));
						++index;
					}
					dataNum = ppAryData[0]->GetSize();
					//ִ�� û�оͲ���  �о͸���
					CStringA strSql = "Replace  INTO `device_info` (`Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,`SynchronState`,`CreateDate`,`CreateUserId` ) VALUES(?,?,?,?,?,?,?,?,?,?);";
					//strSql = "insert into device_type(Id,Name) values (?,?);";
					BOOL ret = sq.QuickInsertData(strSql
						, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);

					for (int index = 0; index < dataCount; ++index)
					{
						delete ppAryData[index];
					}

					delete[] ppAryData;


					sq.CloseDataBase();

					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
						return FALSE;
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}

		//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
		strSql.Format(_T("SELECT `Id` FROM `device_info` where AreaId='%s' and SynchronState='2' \
	and CreateDate >= '%s' and CreateDate <= '%s';"), strAreaID, strBeginTime, strCurTime);
		vecData.clear();
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//�����ݿ���ͬ��
			{
				CSQLite sq;
				if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					int index = 0;
					for each (std::vector<string> varVec in vecData)
					{
						// 					strTemp.Format(("UPDATE device_type set SynchronState ='1' where Id='%s';")
						// 						, (varVec[device_type_Id].c_str()));
						// 					m_updateMysqlData.AddTail(strTemp.GetBuffer());
						ppAryData[index] = new CStringArray;
						ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[device_type_Id].c_str()));

						++index;
					}
					dataNum = ppAryData[0]->GetSize();
					BOOL ret = sq.QuickDeletData("delete from device_info where Id=?", ppAryData
						, dataCount, dataNum, CSQLite::sqlite3_bind);
					for (int index = 0; index < dataCount; ++index)
					{
						delete ppAryData[index];
					}
					delete[] ppAryData;


					sq.CloseDataBase();

					if (!ret)
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
						return FALSE;
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{

			CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}
		CLogRecord::WriteRecordToFile(_T("ͬ����[device_info]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}

/*ͬ��device_type������,
1.pc����phone��ͬ��δͬ��������, ���� ͬ��״̬.
2.����ͬ��״̬Ϊ ��ɾ�� �� ɾ��phone������

strAreaID  ѡ������ID
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_device_type(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;
		strSql.Format(_T("SELECT `device_type`.`Id`,\
		`Name`,\
		`ParentId`,\
		`AreaId`,\
		`SynchronState`,\
		`CreateDate`,\
		`CreateUserId`\
		FROM `device_type` where AreaId='%s' and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strAreaID, strBeginTime, strCurTime);
		//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//�����ݿ���ͬ��
			{
				CSQLite sq;
				if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					int index = 0;

					for each (std::vector<string> varVec in vecData)
					{
						//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
						strTemp.Format(("UPDATE device_type set SynchronState ='1' where Id='%s';")
							, (varVec[device_type_Id].c_str()));
						m_updateMysqlData.AddTail(strTemp.GetBuffer());
						ppAryData[index] = new CStringArray;
						for each (string var in varVec)
						{
							ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
						}
						++index;
					}
					dataNum = ppAryData[0]->GetSize();
					//ִ�� û�оͲ���  �о͸���
					CStringA strSql = "Replace  INTO `device_type` (`Id`,`Name`,`ParentId`,`AreaId`,`SynchronState`,`CreateDate`,`CreateUserId`) VALUES(?,?,?,?,?,?,?);";
					//strSql = "insert into device_type(Id,Name) values (?,?);";
					BOOL ret = sq.QuickInsertData(strSql
						, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
					for (int index = 0; index < dataCount; ++index)
					{
						delete ppAryData[index];
					}
					delete[] ppAryData;


					sq.CloseDataBase();

					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
						return FALSE;
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}

		//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
		strSql.Format(_T("SELECT `Id` FROM  `device_type` where AreaId='%s' and SynchronState='2';"), strAreaID);
		vecData.clear();
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//�����ݿ���ͬ��
			{
				CSQLite sq;
				if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					int index = 0;
					for each (std::vector<string> varVec in vecData)
					{
						// 					strTemp.Format(("UPDATE device_type set SynchronState ='1' where Id='%s';")
						// 						, (varVec[device_type_Id].c_str()));
						// 					m_updateMysqlData.AddTail(strTemp.GetBuffer());
						ppAryData[index] = new CStringArray;
						ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[device_type_Id].c_str()));

						++index;
					}
					dataNum = ppAryData[0]->GetSize();
					BOOL ret = sq.QuickDeletData("delete from device_type where Id=?", ppAryData
						, dataCount, dataNum, CSQLite::sqlite3_bind);
					for (int index = 0; index < dataCount; ++index)
					{
						delete ppAryData[index];
					}
					delete[] ppAryData;


					sq.CloseDataBase();

					if (!ret)
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
						return FALSE;
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{

			CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}
		CLogRecord::WriteRecordToFile(_T("ͬ����[device_type]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}

//��ʼ����������豸����
BOOL CWPD_MTP_dataDlg::Init_area_devices(CString const& strAreaID)
{
	try
	{
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;
		strSql.Format(_T("SELECT `Id` FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s';")
			, strAreaID, strBeginTime, strCurTime);
		//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//�����ݿ���ͬ��
			{
				for each (std::vector<string> varVec in vecData)
				{
					//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�

					m_areasDeviceID.AddTail(varVec[0].c_str());

				}
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}

/*ͬ��sys_user������,
1.pc����phone��ͬ��δͬ��������, ���� ͬ��״̬.
2.����ͬ��״̬Ϊ ��ɾ�� �� ɾ��phone������

��Ҫ���� ѡ������ ���豸ID ����
strAreaID  ѡ������ID ��ʱ����
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_sys_user(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{

		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;

		POSITION posDeviceID = m_areasDeviceID.GetHeadPosition();
		CString strDeviceID;
		int sqlNum = 0;

			strSql.Format(_T("SELECT u.id,u.user_name,u.real_name,u.password,r.role_name,a.Name, u.SynchronState ,u.pwd_key FROM `sys_user` u , sys_role r, areas a\
		where  u.SynchronState='0' and u.data_areas = '%s' and u.data_areas = a.Id and u.role_id = r.id;\
		"), strAreaID);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
							strTemp.Format(("UPDATE sys_user set SynchronState ='1' where id='%s';")
								, (varVec[sys_user_Id].c_str()));
							m_updateMysqlData.AddTail(strTemp.GetBuffer());
							ppAryData[index] = new CStringArray;
							for each (string var in varVec)
							{
								ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
							}
							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						//ִ�� û�оͲ���  �о͸���
						CStringA strSql = "Replace  INTO `user_table` (`Id`,`LoginName`,`Name`,`Password`,`Role`,`AreaName`,`SynchronState`,pwd_key) VALUES(?,?,?,?,?,?,?,?);";

						BOOL ret = sq.QuickInsertData(strSql
							, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}
						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `id` FROM  `sys_user` where SynchronState='2' and data_areas='%s';\
		"), strAreaID);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_template_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sq.QuickDeletData("delete from user_table where id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);
						if (ret)
						{
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
						}
						delete[] ppAryData;


						sq.CloseDataBase();

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

		


		CLogRecord::WriteRecordToFile(_T("ͬ����[sys_user]�������!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}

}


/************************************************************************/
/*
	ִ������:	1.�ȴ��м�sqlite ���ݿ��� ��ȡ��Ҫ���µ�����,���뵽pc��mysql���ݿ���,
				2.��pc��mysql���ݿ��е� ��Ҫ���µ����� �ó��� ���뵽�м�sqlite���ݿ���

				ͬ������ �������� .ֻͬ��ѡ��������
				����ʵ�� �ֱ���.
*/                                                                     
/************************************************************************/
BOOL  CWPD_MTP_dataDlg::BeginSwitchData(CString const& strPath)
{
	try
	{
		//��ʼ���м����
		m_updateMysqlData.RemoveAll();

		m_areasDeviceID.RemoveAll();

		//����mysql���ݿ�
		if (CMyDataBase::GetInstance()->InitMyDataBase(m_mysqlLogin))
		{
			//ѡ��ĵ�ǰ ����ID
			CString strAreaID;
			//CString strAreaName;
			//m_combox_chooseArea.GetLBText(m_combox_chooseArea.GetCurSel(), strAreaName);

			strAreaID = m_mapTreeCtrToID[m_treeCtrl_curItem];


			ASSERT(!strAreaID.IsEmpty());

			BOOL ret = TRUE;
			
			//0.��ʼ����������豸����
			if (!Init_area_devices(strAreaID))
			{
				ShowLog(_T("��ʼ����������豸����ʧ��-ͬ��ʧ��!"));
				ret = FALSE;
			}


			//1.ͬ���� device_type ��Ϣ
			if (!Synchrodata_device_type(strAreaID, strPath))
			{
				ShowLog(_T("�豸���ͱ�(device_type) -ͬ��ʧ��!"));
				ret = FALSE;

			}
			//2.ͬ���� device_info
			if (!Synchrodata_device_info(strAreaID, strPath))
			{
				ShowLog(_T("�豸���ͱ�(device_info) -ͬ��ʧ��!"));
				ret = FALSE;

			}
			//3.ͬ���� work_template
			//ģ��� ����work_type�� ֻͬ�� work_type ��Ҫͬ�������� ������ģ��
			if (!Synchrodata_work_template(strAreaID, strPath))
			{
				ShowLog(_T("�豸���ͱ�(work_template) -ͬ��ʧ��!"));
				ret = FALSE;

			}
			//4.ͬ���� work_type
			if (!Synchrodata_work_type(strAreaID,strPath))
			{
				ShowLog(_T("�豸���ͱ�(work_type) -ͬ��ʧ��!"));
				ret = FALSE;

			}
		

			//5.ͬ���� work_task
			if (!Synchrodata_work_task(strAreaID, strPath))
			{
				ShowLog(_T("�豸���ͱ�(work_task) -ͬ��ʧ��!"));
				ret = FALSE;

			}

			//6.ͬ���� work_record
			if (!Synchrodata_work_record(strAreaID, strPath))
			{
				ShowLog(_T("�豸���ͱ�(work_record) -ͬ��ʧ��!"));
				ret = FALSE;

			}
			//7.ͬ���� sys_user ��Ϣ
			if (!Synchrodata_sys_user(strAreaID, strPath))
			{
				ShowLog(_T("�豸���ͱ�(sys_user) -ͬ��ʧ��!"));
				ret = FALSE;

			}
			
			//00.ͬ����ɺ�,����mysql ͬ��״̬
			if (!UpdateMysqlDB(m_updateMysqlData))
			{
				ShowLog(_T("���·�������ݿ� ͬ��״̬ʧ�� -ͬ��ʧ��!"));
				ret = FALSE;

			}
			CMyDataBase::GetInstance()->Close();
			return ret;
		}
		else
		{
			ShowLog(_T("���ӷ�������ݿ�ʧ��!"));
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}

		
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}


//�ɹ���pc��mysql���ݿ� �������� д���м�sqlite���ݿ�, ���Ƹ����ݿ⵽ phone��
BOOL  CWPD_MTP_dataDlg::BeginPcToPhone(IPortableDevice* & pDevice)
{
	BOOL ret = FALSE;

	WCHAR buffErr[1024] = { 0 };
	//ɾ���ֳ��豸�ϵ�phone.db
	DeleteDataFromDevice(pDevice, gFileID);
// 	UpdateContentOnDevice(pDevice,
// 		WPD_CONTENT_TYPE_FOLDER,
// 		L"*.*\0*.*\0\0",
// 		nullptr, gDirID);
	//����phone.db���ֳ��豸��
	//gFilePath = _T(".//phone.db");
	
	ret = TransferDataToDevice(pDevice, WPD_CONTENT_TYPE_ALL, gDirID,gFilePath, buffErr);
	
	CLogRecord::WriteRecordToFile(buffErr);
	//ɾ����ǰ���м��ļ�
	DeleteFile(gFilePath);

	return ret;
}

//�ȴ��豸 ����,���Ӻ� ��ȡָ��·���µ��ļ�,�����ݵ�pc��
BOOL CWPD_MTP_dataDlg::BeginPhoneToPc(IPortableDevice* & pDevice)
{
	
	int curIndex = m_combox_devices.GetCurSel();
	if (curIndex < 0)
	{
		ShowLog(_T("��ѡ��һ���豸"));
		return FALSE;
	}
	ASSERT(curIndex >= 0);
	ASSERT(curIndex < m_strDevicesID.GetSize());
	CString strDevID = m_strDevicesID.GetAt(curIndex); //��ǰѡ����豸ID
													   //1.�ȴ��豸����
													   //2.��ȡphone�˵��豸ָ��·���µ�sqlite�豸 
													   //(1)���ݵ����� (2)��ȡ���� ��Ҫͬ��������
	HRESULT hr = S_OK;

	gDirID = _T(""), gFileID = _T("");
	//gDevice = nullptr;
	gFilePath = _T(".//") + m_fileName; //�м��ļ�·��
	////////////////////////////////////
	WCHAR buffErr[1024] = { 0 };
	if (!OpenDevice(strDevID, &pDevice) && pDevice == nullptr)
	{
		ShowLog(_T("�豸����ʧ��,��ȡ�豸ʧ��!"));
		return FALSE;
	}


	IPortableDeviceContent*  content=nullptr;
	hr = pDevice->Content(&content);
	if (FAILED(hr))
	{
		CString strLog;
		strLog.Format(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
		ShowLog(strLog);
		return FALSE;
	}

	CString dirID, fileID; //���һ���ļ��� ID �� ���յ����ݿ��ļ�ID
	//��ȡsqlite���ݿ��ļ���ID
	CString strPath;
	for (int index =0; index < m_aryFileName.GetSize();++index)
	{
		strPath += m_aryFileName.GetAt(index) + _T("#");
		if (index ==0) //��һ��
		{
			PCWSTR objID = getSpecifiedObjectID(pDevice, m_aryFileName.GetAt(index));
			dirID = objID;
			continue;;
		}
	
		if (index == 1) //�ڶ���
		{
			PCWSTR objID = getIDByParentID(pDevice, content, dirID, m_aryFileName.GetAt(index));
			fileID = objID;
			continue;
		}
		//����2��֮��
		PCWSTR objID = getIDByParentID(pDevice, content, fileID, m_aryFileName.GetAt(index));

		dirID = fileID;
		fileID = objID;
	}
	if (dirID.IsEmpty() || fileID.IsEmpty())
	{
		ShowLog(_T("��ʧ���ݿ��ļ�,��ȷ���ļ�����!----"));
		if (content)
		{
			content->Release();
		}
		return FALSE;
	}
	//���浱ǰʹ���ļ�ID
	gDirID = dirID;

	gFileID = fileID;
	
	TransforDataFromDevice(pDevice, fileID);
	//(1)��ɾ�����ص��м��ļ�, ����phone ���ļ�, ����sqlite ���ݿ� 


	CTime Time = CTime::GetTickCount();
	CString strName = Time.Format(_T("%Y-%m-%d-%H-%M"));
	BOOL bRet = CopyFile(_T(".//") + m_fileName, _T(".//backUp//") + strName, FALSE);
	gFilePath = _T(".//") + m_fileName; //�м��ļ�·��
	ShowLog(_T("����ͬ��..."));
	if (content)
	{
		content->Release();
	}

	return TRUE;
}
/************************************************************************/
/*
ע�����:WPDBusEnum  (Portable Device Enumerator Service)

ͬ������:
	˵��:���ص����и�mysql���ݿ�,usb���ӵ�android�ֻ��� ��һ��sqlite ���ݿ�.
ͬ�����ܷ�Ϊ1.phone�� ͬ��sqlite�����µ����� ��pc�� mysql���ݿ� 2.pc�� mysql �����µ����� ͬ���� 
phone�� sqlite ���ݿ���.

	ִ������:1.�̵߳ȴ� phone�豸ͨ��usb��ʽ���� pc��. 2.�ȵ���ȷ���豸����,��ȡphone�˵�
	sqlite ���ݿ�,������pc��.(1)������.��ʱ��ڵ�Ϊ����.(2)��sqlite�������µ�����д��pc��mysql���ݿ�
	(3)��ȡpc�˵�mysql���ݿ��� ��Ҫ���µ�����.��д��sqlite.(4)ɾ��phone�˵�sqlite���ݿ�,���µ�sqlite���ݿ�
	���ƹ�ȥ.

*/
/************************************************************************/
UINT MyControllingFunction(LPVOID pParam)
{
	//����ָ��
	CWPD_MTP_dataDlg* pDlg = (CWPD_MTP_dataDlg*)pParam;
	IPortableDevice* gDevice = nullptr; //����ʹ�õ��豸

	/************************************************************************/
#ifdef DEBUG
	CTime tm;
	tm = CTime::GetCurrentTime();
	pDlg->ShowLog(tm.Format(_T("%H%M%S")));
#endif
#ifdef MYTEST
	gFilePath = _T(".//phone.db");
#endif
	//1.����phone���ݵ� pc����
	BOOL ret = FALSE;
	 if (pDlg->BeginPhoneToPc(gDevice))
	 {
		 
		 //2.ͬ�����µ�sqlite���� ��pc mysql���ݿ�  ����ȡ���µ�mysql���� д��sqlite ���ݿ�
		 if (pDlg->BeginSwitchData(gFilePath))
		 {
			// 3.����phone�˵����ݿ�  ��pc�� �м�sqlite���ݿ� ������ phone��ָ��λ��
			 ret =  pDlg->BeginPcToPhone(gDevice);
		 }
	 }
	 if (gDevice)
	 {
		 gDevice->Close();
		 gDevice->Release();
	 }

	

	pDlg->m_threadFlag = FALSE;
	if (ret)
	{
		pDlg->ShowLog(_T("ͬ�����..."));
#ifdef DEBUG
		tm = CTime::GetCurrentTime();
		pDlg->ShowLog(tm.Format(_T("%H%M%S")));
#endif
	}
	else
		pDlg->ShowLog(_T("ͬ��ʧ��..."));
	return 0;
	/************************************************************************/
}
void CWPD_MTP_dataDlg::OnBnClickedBtTophone()
{	
	//�ж� ���ؼ�ѡ�� ΪҶ�ڵ�
	if (nullptr == m_treeCtrl_curItem)
	{
		ShowLog(_T("��ѡ��һ������"));
		return;
	}
	//����
#ifdef MYTEST
	//gFilePath = _T(".//phone.db");
	BeginSwitchData(gFilePath);
	return;
#endif

	if (!m_threadFlag)
	{
		// TODO: �ڴ���ӿؼ�֪ͨ����������
		AfxBeginThread(MyControllingFunction, this);
		m_threadFlag = TRUE;
	}
	else
		ShowLog(_T("��������Ƶ��!"));
}
LRESULT CWPD_MTP_dataDlg::OnMyDeviceChange(WPARAM wParam, LPARAM lParam)
{
	if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) {
		PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
		PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
		PDEV_BROADCAST_HANDLE pDevHnd;
		PDEV_BROADCAST_OEM pDevOem;
		PDEV_BROADCAST_PORT pDevPort;
		PDEV_BROADCAST_VOLUME pDevVolume;
		switch (pHdr->dbch_devicetype) {
		case DBT_DEVTYP_DEVICEINTERFACE:
		{
			//��Ҫ�Ĳ���  
			ASSERT(m_threadShowDevs);
			ResumeThread(m_threadShowDevs->m_hThread);
			
		}
		break;

		case DBT_DEVTYP_HANDLE:
			pDevHnd = (PDEV_BROADCAST_HANDLE)pHdr;
			break;

		case DBT_DEVTYP_OEM:
			pDevOem = (PDEV_BROADCAST_OEM)pHdr;
			break;

		case DBT_DEVTYP_PORT:
			pDevPort = (PDEV_BROADCAST_PORT)pHdr;
			break;

		case DBT_DEVTYP_VOLUME:
			pDevVolume = (PDEV_BROADCAST_VOLUME)pHdr;
			break;
		}
	}
	return 0;


}

void CWPD_MTP_dataDlg::OnCbnSelchangeComboxDevices()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strRee = _T("");
	return;
}


void CWPD_MTP_dataDlg::OnInputDeviceChange(unsigned short nState, HANDLE hDevice)
{
	// �˹���Ҫ�� Windows Vista ����߰汾��
	// _WIN32_WINNT ���ű��� >= 0x0600��
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnInputDeviceChange(nState, hDevice);
}


void CWPD_MTP_dataDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	
	CDialogEx::OnOK();
}


void CWPD_MTP_dataDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	//�������м��ļ�,��ɾ��
	if (!gFilePath.IsEmpty())
	{
		//ɾ����ǰ���м��ļ�
		DeleteFile(gFilePath);
	}
	//�رմ򿪵��豸


	m_threadFlag = FALSE;
	ASSERT(m_threadShowDevs);
	ResumeThread(m_threadShowDevs->m_hThread);
	//CMyDataBase::GetInstance()->Close();
	CDialogEx::OnClose();
}


void CWPD_MTP_dataDlg::OnLbnDblclkListMsg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_listShow.ResetContent();
}


void CWPD_MTP_dataDlg::OnTvnSelchangedTreeAreas(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	m_treeCtrl_curItem = pNMTreeView->itemNew.hItem;
	m_static_current_area.SetWindowText(m_mapArea[m_mapTreeCtrToID[m_treeCtrl_curItem]]);
}


void CWPD_MTP_dataDlg::OnBnClickedRefreshDevs()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(m_threadShowDevs);
	ResumeThread(m_threadShowDevs->m_hThread);
	ShowLog(_T("ɨ���豸��Ϣ,��ʧ��,�����²��һ���豸!"));
}
