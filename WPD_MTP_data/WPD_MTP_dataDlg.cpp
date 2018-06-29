
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
void EnumerateAllContent(_In_ IPortableDevice* device, _In_ PCWSTR fileName, _Out_ TCHAR* bufferr);
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
void MoveContentAlreadyOnDevice(_In_ IPortableDevice* device, _In_ PCWSTR pselection
	, _In_ PCWSTR pdestinationFolderObjectID);

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
PCWSTR getSpecifiedObjectID(_In_ IPortableDevice* device, PCWSTR fileName,TCHAR* bufferr);
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
	PCWSTR objID = _T("");// = getSpecifiedObjectID(device.Get(), L"����");  //koudaigouwu.apk
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
	m_deleteTree = nullptr;
}

void CWPD_MTP_dataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MSG, m_listShow);
	DDX_Control(pDX, IDC_COMBOX_Devices, m_combox_devices);
	DDX_Control(pDX, IDC_COMBOX_QyChoose, m_combox_chooseArea);
	DDX_Control(pDX, IDC_TREE_AREAS, m_tree_areas);
	DDX_Control(pDX, IDC_STATIC_Area, m_static_current_area);
	DDX_Control(pDX, IDC_TREE_TYPE, m_tree_type);
	DDX_Control(pDX, IDC_STATIC_CurChoose, m_static_curchoose);
	DDX_Control(pDX, IDC_BUTTON_ZDTOPC, m_button_zdtopc);
	DDX_Control(pDX, IDC_BT_TOPHONE, m_bt_tophone);
	DDX_Control(pDX, IDC_DATETIMEPICKER_BEGIN, m_datetime_begin);
	DDX_Control(pDX, IDC_DATETIMEPICKER_END, m_datetime_end);
	DDX_Control(pDX, IDC_BUTTON_BEGIN_CLEAR, m_bt_begin_clear);
	DDX_Control(pDX, IDC_BUTTON_REBACK, m_bt_begin_reback);
	DDX_Control(pDX, IDC_STATI_TREE_SHOW, m_static_tree_show);
	DDX_Control(pDX, IDC_REFRESH_DEVS, m_button_flash);
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
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_TYPE, &CWPD_MTP_dataDlg::OnTvnSelchangedTreeType)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TREE_AREAS, &CWPD_MTP_dataDlg::OnNMCustomdrawTreeAreas)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TREE_TYPE, &CWPD_MTP_dataDlg::OnNMCustomdrawTreeType)
	ON_BN_CLICKED(IDC_BUTTON_ZDTOPC, &CWPD_MTP_dataDlg::OnBnClickedButtonZdtopc)
	ON_BN_CLICKED(IDC_RADIO_USRS, &CWPD_MTP_dataDlg::OnBnClickedRadioUsrs)
	ON_BN_CLICKED(IDC_RADIO_DEVICES, &CWPD_MTP_dataDlg::OnBnClickedRadioDevices)
	ON_BN_CLICKED(IDC_RADIO_WORK, &CWPD_MTP_dataDlg::OnBnClickedRadioWork)
	ON_BN_CLICKED(IDC_BUTTON_BEGIN_CLEAR, &CWPD_MTP_dataDlg::OnBnClickedButtonBeginClear)
	ON_BN_CLICKED(IDC_BUTTON_REBACK, &CWPD_MTP_dataDlg::OnBnClickedButtonReback)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_END, &CWPD_MTP_dataDlg::OnDtnDatetimechangeDatetimepickerEnd)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_BEGIN, &CWPD_MTP_dataDlg::OnDtnDatetimechangeDatetimepickerBegin)

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

UINT Init_areas_tree_thread(LPVOID pParam);
//���ؼ�ѡ�� ��ʾ
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

	//��ʼ����Դ
	if (!InitResource())
	{
		ShowLog(_T("��Դ��ʼ��ʧ��!"));
	}


	//��ʼ������tree
	AfxBeginThread(Init_areas_tree_thread, this);

	//Ĭ��ѡ������
	((CButton*)GetDlgItem(IDC_RADIO_WORK))->SetCheck(1);
	m_radioChoose = radio_work;

	//���ȼ�Ϊ��һ�㡱���̣߳�Ĭ��ջ��С������ʱ���� CREATE_SUSPENDED
	m_threadShowDevs = AfxBeginThread(ShowAllDevices, this,THREAD_PRIORITY_NORMAL, 0);


	//���ô��ڱ��� �û�������Ч
	if (!m_userRealName.IsEmpty())
	{
		SetWindowText(_T("���ݹ��� -- �û���:") + m_userRealName);
	}
	

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

//�����û�������
bool CWPD_MTP_dataDlg::SetUserAreaRealName(CString const& strArea,CString const& strName)
{
	m_userArea = strArea;
	m_userRealName = strName;
	return TRUE;
}
//��ʼ��ͼƬ��Դ
bool CWPD_MTP_dataDlg::InitResource()
{
#ifndef TREESHOWICON
	return true;
#endif
	m_treeImageList.Create(32, 32, ILC_COLOR32, 3, 3);

// 	m_treeIconList[tree_system] = (HICON)::LoadImage(NULL, CLogRecord::GetAppPath() + _T("//resource//system.ico")
// 		, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
// 	m_treeImageList.Add(m_treeIconList[tree_system]);
// 
// 	m_treeIconList[tree_device] = (HICON)::LoadImage(NULL, CLogRecord::GetAppPath() + _T("//resource//device.ico")
// 		, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
// 	m_treeImageList.Add(m_treeIconList[tree_device]);
	CBitmap  imgAreas;
	imgAreas.Attach(::LoadImage(NULL, CLogRecord::GetAppPath() + _T("//resource//����.bmp")
		 		, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	CBitmap  imgDevices;
	imgDevices.Attach(::LoadImage(NULL, CLogRecord::GetAppPath() + _T("//resource//�豸.bmp")
		, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	m_treeImageList.Add(&imgAreas, RGB(0,0,0));
	m_treeImageList.Add(&imgDevices, RGB(0, 0, 0));
	if (m_treeImageList.GetImageCount()>0)
	{
		m_tree_areas.SetImageList(&m_treeImageList, TVSIL_NORMAL);  // ���� imagelist �� tree��ӳ���ϵ
		return true;
	}
	return false;
}

//��־���
void CWPD_MTP_dataDlg::ShowLog(CString const& strLog)
{
	m_listShow.AddString(strLog);
	CLogRecord::WriteRecordToFile(strLog);
	CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
}

//Ѱ�� �ӽڵ� �� ���뵽���ؼ�
//����Ϊ ���ڵ� �� ��Ӧ���ؼ��� ���
void  CWPD_MTP_dataDlg::FindChildTreeType(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent)
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
			POSITION pos = m_mapTypeParent.GetStartPosition();
			while (pos)
			{
				m_mapTypeParent.GetNextAssoc(pos, strKey, strValue);//���� ���е� �ڵ�				
				if (parentKey == strValue) //���ýڵ�Ϊ���ڵ�
				{
					HTREEITEM item = m_tree_areas.InsertItem(m_mapType[strKey],1,1, valueItem);// �ڸ���������Parent
					mapParentItem[strKey] = item; //�����µ� ���ڵ�
					m_mapTreeCtrToIDType[item] = strKey;
				}
			}
			//�ݹ� ��ȡ
			FindChildTreeType(mapParentItem);
		}
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
	CString strKey, strValue;
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
				HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey],0,0, valueItem);// �ڸ���������Parent
				mapParentItem[strKey] = item; //�����µ� ���ڵ�
				if (item != nullptr)
				{
					m_mapTreeCtrToID[item] = strKey;
					m_mapTreeIDToCtrl[strKey] = item;
				}
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
			sqlNum = 0;
			continue;
		}
		else
		{
			strTemp += list.GetNext(posUpdate);
		}
		//pDC->TextOut(200, 200, );//�����������View���е�OnDraw()

	}
	BOOL ret = TRUE;
	if (strTemp != "" && sqlNum == 1)
	{
		ret = CMyDataBase::GetInstance()->OneQuery(strTemp);
	}
	else if (strTemp != "" && sqlNum > 1)
	{
		ret= CMyDataBase::GetInstance()->Query(strTemp);
	}
	if (!ret)
	{
		CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(strTemp.c_str()));
	}
	return ret;
}

//�豸 ������Ӧ���� ����������
BOOL  CWPD_MTP_dataDlg::InitDeviceTree(CString const& strAreaID)
{
	//1.��ʼ������
	m_mapType.RemoveAll();
	m_mapTypeParent.RemoveAll();
	m_mapTypeToArea.RemoveAll();
	m_tree_type.DeleteAllItems();
	m_mapTreeCtrToIDType.RemoveAll(); //����

	std::vector<std::vector<std::string> > vecDataType; //mysql ���ݿ������
														//2.��ȡ mysql���ݿ�� work_type��
	if (CMyDataBase::GetInstance()->InitMyDataBase(m_mysqlLogin))
	{
		CStringA strSql;
		strSql.Format("SELECT `Id`,\
					`ParentId`,\
					`Name`,\
					`AreaId`\
					FROM `device_type` where  SynchronState='0' or  SynchronState='1' and AreaId='%s';", CpublicFun::UnicodeToAsc(strAreaID));
		if (CMyDataBase::GetInstance()->Select(strSql.GetBuffer(), vecDataType))
		{
			int index = 0;
			CString strName;
			for each (std::vector<std::string> varVec in vecDataType)
			{
				// 					CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapType; //����ѡ��
				// 					CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParent; //���� ��Ӧ�� ����ID
				// 					CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToIDType; //���ؼ���Ӧ�Ľڵ�ID
				//����ѡ�� ����ID
				strName = CpublicFun::AscToUnicode(varVec[work_type_tree_Name].c_str());
				m_mapType[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())] = strName;
				//m_combox_chooseArea.AddString(strName);
				m_mapTypeParent[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
					= CpublicFun::AscToUnicode(varVec[work_type_tree_ParentId].c_str());
				m_mapTypeToArea[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
					= CpublicFun::AscToUnicode(varVec[work_type_tree_AreaId].c_str());
			}

			//m_combox_chooseArea.SetCurSel(0);
		}
		else
		{
			CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err+ strSql));
			ShowLog(_T("mysql���ݿ�����ʧ��,��ȷ�����������������!"));
			//�ر����ݿ�
			CMyDataBase::GetInstance()->Close();
			return FALSE;
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
	//work_type ��������
	POSITION posType = m_mapTypeParent.GetStartPosition();
	CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItemType; //���ڵ� ��Ӧ�� ���ؼ��ڵ� ���
	CString strKeyType, strValueType;
	while (posType)
	{
		m_mapTypeParent.GetNextAssoc(posType, strKeyType, strValueType);
		if (strValueType == PARENTFLAG) //Ϊ���׽ڵ�
		{
			HTREEITEM itemType = m_tree_type.InsertItem(m_mapType[strKeyType], TVI_ROOT);// �ڸ���������Parent
			mapParentItemType[strKeyType] = itemType;
			m_mapTreeCtrToIDType[itemType] = strKeyType;

		}
	}
	FindChildTreeType(mapParentItemType);
	return TRUE;
}


//��ȡ������ص�work_type  ��Ϣ
BOOL  CWPD_MTP_dataDlg::InitWorkTypeMap(CString const& strAreaID,CStringArray & strAryID)
{

	std::vector<std::vector<std::string> > vecDataType; //mysql ���ݿ������
														//2.��ȡ mysql���ݿ�� work_type��
	//if (CMyDataBase::GetInstance()->InitMyDataBase(m_mysqlLogin))
	{
		CStringA strSql;
		strSql.Format("SELECT `Id`,\
					`ParentId`,\
					`Name`,\
					`AreaId`\
					FROM `device_type` where SynchronState='0' or  SynchronState='1' and AreaId='%s';", CpublicFun::UnicodeToAsc(strAreaID));
		if (CMyDataBase::GetInstance()->Select(strSql.GetBuffer(), vecDataType))
		{
			int index = 0;
			CString strName;
			for each (std::vector<std::string> varVec in vecDataType)
			{
				strAryID.Add(CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str()));
			}

		
		}
		else
		{
			CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err+ strSql));
			ShowLog(_T("mysql���ݿ�����ʧ��,��ȷ�����������������!"));
			//�ر����ݿ�
			//CMyDataBase::GetInstance()->Close();
			return FALSE;
		}
	}
// 	else
// 	{
// 		CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
// 		CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
// 		ShowLog(_T("mysql���ݿ�����ʧ��,��ȷ�����������������!"));
// 		return FALSE;
// 	}
	//�ر����ݿ�
	//CMyDataBase::GetInstance()->Close();

	return TRUE;
}
UINT Init_areas_tree_thread(LPVOID pParam)
{
	//����ָ��
	CWPD_MTP_dataDlg* pDlg = (CWPD_MTP_dataDlg*)pParam;
	pDlg->Init_areas_tree();
	return 0;
}
//��ʼ�� ����ϵͳ(�豸)��
BOOL CWPD_MTP_dataDlg::Init_areas_tree()
{
	try {
		m_mapArea.RemoveAll();
		m_mapAreaParent.RemoveAll();
		m_mapTreeCtrToID.RemoveAll();
		m_mapTreeIDToCtrl.RemoveAll();
		m_tree_areas.DeleteAllItems();
		m_treeCtrl_curItem = nullptr;
		m_treeCtrl_curItemType = nullptr;
		m_static_current_area.SetWindowText(_T(""));
		m_tree_type.DeleteAllItems();


		//device_info
		m_mapType.RemoveAll();
		m_mapTypeParent.RemoveAll();
		m_mapTypeToArea.RemoveAll();
		m_tree_type.DeleteAllItems();
		m_mapTreeCtrToIDType.RemoveAll(); //����

		//����ɾ�����ṹ���м����
		m_mapTypeParentDelete.RemoveAll();
		m_mapTypeToAreaDelete.RemoveAll();
		m_mapAreaParentDelete.RemoveAll();

		//����ˢ�ؼ�
		m_button_flash.EnableWindow(FALSE);
		//////////////////////////////////////////////////////////////////////////
		ShowLog(_T("���ڳ�ʼ�����ؼ�,���Ժ�..."));
		BOOL retRes = TRUE;
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
														//2.��ȡ mysql���ݿ�� �����

		std::vector<std::vector<std::string> > vecDataType; //mysql ���ݿ������
															//2.��ȡ mysql���ݿ�� work_type��
		if (CMyDataBase::GetInstance()->InitMyDataBase(m_mysqlLogin))
		{
			if (CMyDataBase::GetInstance()->Select("SELECT `areas`.`Id`,\
			`areas`.`Code`,\
			`areas`.`Name`,\
			`areas`.`ParentId`,\
			`areas`.`CreateDate`,\
			`areas`.`CreateUserId`\
			,SynchronState\
			FROM `areas` ;", vecData))
			{
				int index = 0;
				CString strName;
				for each (std::vector<std::string> varVec in vecData)
				{
					strName = CpublicFun::AscToUnicode(varVec[areas_Name].c_str());
					//���ؼ�ֻչʾ δɾ������
					if (varVec[areas_SynchronState] == "0" || varVec[areas_SynchronState] == "1")
					{
						//����ѡ�� ����ID
						m_mapArea[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] = strName;
						//m_combox_chooseArea.AddString(strName);
						m_mapAreaParent[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] 
							= CpublicFun::AscToUnicode(varVec[areas_ParentId].c_str());
					}
					m_mapAreaParentDelete[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] 
						= CpublicFun::AscToUnicode(varVec[areas_ParentId].c_str());
					m_mapAreaStatusDelete[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] 
						= CpublicFun::AscToUnicode(varVec[areas_SynchronState].c_str());
					m_mapAreaNameDelete[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] = strName;
			
				}
			}
			else
			{
				CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
				ShowLog(_T("mysql���ݿ�����ʧ��,��ȷ�����������������!"));
				retRes &= FALSE;
			}
			//////////////////////////////////////////////////////////////////////////
			//device_info
			CStringA strSql;
			strSql.Format("SELECT `Id`,\
					`ParentId`,\
					`Name`,\
					`AreaId`,\
					SynchronState\
					FROM `device_info`;");
			if (CMyDataBase::GetInstance()->Select(strSql.GetBuffer(), vecDataType))
			{
				int index = 0;
				CString strName;
				for each (std::vector<std::string> varVec in vecDataType)
				{
				
					strName = CpublicFun::AscToUnicode(varVec[work_type_tree_Name].c_str());
					//���ؼ�ֻչʾ δɾ������
					if (varVec[work_type_tree_SynchronState] == "0"
						|| varVec[work_type_tree_SynchronState] == "1")
					{
						//����ѡ�� ����ID
						
						m_mapType[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())] = strName;
						//m_combox_chooseArea.AddString(strName);
						m_mapTypeParent[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
							= CpublicFun::AscToUnicode(varVec[work_type_tree_ParentId].c_str());
						m_mapTypeToArea[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
							= CpublicFun::AscToUnicode(varVec[work_type_tree_AreaId].c_str());
					}
					m_mapTypeParentDelete[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
						= CpublicFun::AscToUnicode(varVec[work_type_tree_ParentId].c_str());
					m_mapTypeToAreaDelete[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
						= CpublicFun::AscToUnicode(varVec[work_type_tree_AreaId].c_str());	
					m_mapTypeStatusDelete[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
						= CpublicFun::AscToUnicode(varVec[work_type_tree_SynchronState].c_str());
					m_mapTypeNameDelete[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
						= strName;
		
					
				}
			}
			else
			{
				CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
				retRes &= FALSE;
			}
			//�ر����ݿ�
			CMyDataBase::GetInstance()->Close();

		}
		else
		{
			CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
			
			retRes &= FALSE;
		}


		m_button_flash.EnableWindow(TRUE);
		//����������豸������ ʧ�� �����쳣
		if (!retRes)
		{
			ShowLog(_T("�����쳣,mysql���ݿ�����ʧ��,������!"));
			return retRes;
		}
		

		//1.����
		{
			//////////////////////////////////////////////////////////////////////////
			//������
			DWORD dwStyles = m_tree_areas.GetStyle();//��ȡ������ԭ���  
			dwStyles |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;//
			m_tree_areas.ModifyStyle(0, dwStyles);


			//��ʼ�� ���ؼ�
			//1.�Ȳ��� ���׽ڵ�
			POSITION pos = m_mapAreaParent.GetStartPosition();
			CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItem; //���ڵ� ����ID��Ӧ�� ���ؼ��ڵ� ���
			CString strKey, strValue;
			while (pos)
			{
				m_mapAreaParent.GetNextAssoc(pos, strKey, strValue);
				//�û�����ǿ�
				if (!m_userArea.IsEmpty())
				{
					if (strKey == m_userArea)
					{
						HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey], 0, 0, TVI_ROOT);// �ڸ���������Parent
						mapParentItem[strKey] = item;
						if (item != nullptr)
						{
							m_mapTreeCtrToID[item] = strKey;
							m_mapTreeIDToCtrl[strKey] = item;
						}
					}
				}
				else if (strValue == PARENTFLAG) //Ϊ���׽ڵ�
				{
					HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey], 0, 0, TVI_ROOT);// �ڸ���������Parent
					mapParentItem[strKey] = item;
					if (item != nullptr)
					{
						m_mapTreeCtrToID[item] = strKey;
						m_mapTreeIDToCtrl[strKey] = item;
					}
					//m_mapAreaToCtr[strKey] = item; //�������� ���������ؼ� �ڵ�
				}
			}
			//�ݹ鹹�� ���� ��
			FindChildTree(mapParentItem);


			//////////////////////////////////////////////////////////////////////////
		
			//�豸����
			//device_info �豸���������
			//1.�豸Ϊ���ڵ�����
			POSITION posType = m_mapTypeParent.GetStartPosition();
			CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItemType; //���ڵ� ��Ӧ�� ���ؼ��ڵ� ���
			CString strKeyType, strValueType;
			HTREEITEM itemType = nullptr;
			while (posType)
			{
				m_mapTypeParent.GetNextAssoc(posType, strKeyType, strValueType);
				itemType = nullptr; //�����м����
				if (strValueType == PARENTFLAG) //Ϊ���׽ڵ�
				{
					if (m_mapTypeToArea[strKeyType] != _T("")) //��ʾ������
					{
						//������Ӧ����Ľڵ�
						if (m_mapTreeIDToCtrl[m_mapTypeToArea[strKeyType]] != nullptr)
						{
							itemType = m_tree_areas.InsertItem(m_mapType[strKeyType], 1, 1, m_mapTreeIDToCtrl[m_mapTypeToArea[strKeyType]]);// �ڸ���������Parent
						}
					}
					else
						itemType = m_tree_areas.InsertItem(m_mapType[strKeyType], 1, 1, TVI_ROOT);// �ڸ���������Parent
					if (itemType) //�ɹ�������һ���ڵ�
					{
						mapParentItemType[strKeyType] = itemType;
						m_mapTreeCtrToIDType[itemType] = strKeyType;
					}
					
				}
			}
			FindChildTreeType(mapParentItemType);

			//2.�豸Ϊ�ӽڵ�����
			//FindChildTreeType(m_mapTreeIDToCtrl);

			HTREEITEM hCurItem = m_tree_areas.GetRootItem();

			//չ�����нڵ�
			while (hCurItem)
			{
				MyExpandTree(m_tree_areas, hCurItem);
				hCurItem = m_tree_areas.GetNextSiblingItem(hCurItem);
			}
			m_tree_areas.SetScrollPos(0, 0); //ˮƽ������
			m_tree_areas.SetScrollPos(1, 0); //��ֱ������

		}

		//2.��ʼ��ɾ�������ṹ
		retRes =InitDeleteTreeStruct();

		//3.��ʼ�����
		ShowLog(_T("��ʼ�����ؼ����!"));
		return retRes;
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
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_record(CStringArray const&  strAryAreaID, CString const& strDBPath, int tongbuDir)
{
	try {
		//0.��ʼ����ɾ������
		if (!PathFileExists(strDBPath))
		{
			CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
			return FALSE;
		}

		CString strDeviceID;
		/*
		ƴ���ַ���
		*/
		CString strTemp;
		for (int index = 0; index <strAryAreaID.GetSize(); ++index)
		{
			//��һ������
			if (index == 0)
			{
				strTemp.Format(_T("'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			else
			{
				strTemp.Format(_T(",'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}

		}


		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
		CString strSql;
		CSQLite sqOne;
		BOOL retRes = TRUE;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			//���ն�ͬ����pc
			if (tongbuDir == tongbu_to_pc)
			{
				//0.ͬ��phone�˵ļ�¼

				CList<CStringA> listUpdateSql; //������Ҫ���µ�sql���

				{
					int RFidCount = 0, RFidNum = 0;
					BOOL ret = FALSE;
					strSql.Format(_T("SELECT count(*) FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId in(%s);")
						, strBeginTime, strCurTime, strDeviceID);
					//char buf[MAX_PATH] = { 0 };
					if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
						//if (sqOne.QuickSelectDataCount(strSql,RFidCount))
					{
						//RFidCount = atoi(buf);
						if (RFidCount > 0)
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[RFidCount]; //ͬ����mysql���ݿ�
							for (int index = 0; index < RFidCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							CStringArray** ppAryDataSqlite = new CStringArray*[RFidCount]; //����phone��sqlite ���� Id �� ͬ��״̬
							for (int index = 0; index < RFidCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							strSql.Format(_T("SELECT `Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`Executor`,ClassName FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s' and  DeviceId in(%s);")
								, strBeginTime, strCurTime, strDeviceID);

							if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
							{
								//����ת��

								CString strRepTemp; //�м����
								for (int index = 0; index < RFidCount; ++index)
								{
									if (ppAryData[index] == nullptr)
									{
										continue;
									}
									ppAryDataSqlite[index] = new CStringArray;
									ppAryDataSqlite[index]->Add(ppAryData[index]->GetAt(work_record_Id));


									//������ת��
									for (int ind = 0; ind < work_record_max; ++ind)
									{

										strRepTemp = ppAryData[index]->GetAt(ind);
										if (-1 != strRepTemp.Find(_T("\\")))
										{
											strRepTemp.Replace(_T("\\"), _T("\\\\"));
											ppAryData[index]->SetAt(ind, strRepTemp);
										}

										if (-1 != strRepTemp.Find(_T("'")))
										{
											strRepTemp.Replace(_T("'"), _T("\\'"));
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



								int retFlag = TRUE; //���ݿ����
													//����mysql���ݿ�
								if (!UpdateMysqlDB(listUpdateSql))
								{
									ShowLog(_T("��phone���¼�¼���ݷ����work_recordʧ��!"));
									CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
									retFlag = false;
									retRes &= TRUE;
								}
								else
								{
									RFidNum = ppAryDataSqlite[0]->GetSize(); //��ȡ������
																			 //����phone��sqlite ״̬
									if (!sqOne.QuickInsertData("UPDATE work_record set SynchronState ='1' where Id=?;", ppAryDataSqlite, RFidCount, RFidNum, CSQLite::sqlite3_bind))
									{
										CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
										retFlag = false;
										retRes &= TRUE;
									}
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
								delete[] ppAryData;
								delete[] ppAryDataSqlite;
								if (!retFlag)//����Ĳ������
								{
									retRes &= TRUE;
								}

							}
						}
					}
					
					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + strSql);
						retRes &= TRUE;
					}
				}
			}
			else if (tongbuDir == tongbu_to_phone) //��pcͬ�����ն�
			{

				//ͬ������˵�����

				strSql.Format(_T("SELECT `Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,ClassName FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
					, strBeginTime, strCurTime);
				//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//�����ݿ���ͬ��
					{
					
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[dataCount];
							for (int index = 0; index < dataCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							int index = 0;
							CList<CStringA> updateMysqlData;
							for each (std::vector<string> varVec in vecData)
							{
								//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
								strTemp.Format(("UPDATE work_record set SynchronState ='1'  where Id='%s';")
									, (varVec[work_record_Id].c_str()));
								updateMysqlData.AddTail(strTemp.GetBuffer());
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

							BOOL ret = sqOne.QuickInsertData(strSql
								, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}

							delete[] ppAryData;
							if (!ret)
							{
								CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() 
									+ CpublicFun::AscToUnicode(strSql));
								retRes &= TRUE;
							}
							else //ͬ���ɹ� ����״̬
							{
								POSITION posUpdate = updateMysqlData.GetHeadPosition();
								while (posUpdate != NULL)
								{
									m_updateMysqlData.AddTail(updateMysqlData.GetNext(posUpdate));
								}
							}
						}
					
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					retRes &= FALSE;
				}
			}
			sqOne.CloseDataBase();
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			return FALSE;
		}
		

		CLogRecord::WriteRecordToFile(_T("ͬ����[work_record]�������!"));
		return retRes;
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
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_task(CStringArray const&  strAryAreaID, CString const& strDBPath, int tongbuDir)
{
	try
	{
		//0.��ʼ����ɾ������
		if (!PathFileExists(strDBPath))
		{
			CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
			return FALSE;
		}
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
		CString strSql;

		//�豸ID
		CString strDeviceID;
		/*
		ƴ���ַ���
		*/
		CString strTemp;
		for (int index = 0; index <strAryAreaID.GetSize(); ++index)
		{
			//��һ������
			if (index == 0)
			{
				strTemp.Format(_T("'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			else
			{
				strTemp.Format(_T(",'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}

		}

		int sqlNum = 0;
		CSQLite sqOne;
		BOOL ret = TRUE;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			//���ն�ͬ����pc
			if (tongbuDir == tongbu_to_pc)
			{
				std::vector<std::vector<std::string> > vecDataTime; //mysql ���ݿ������
				strSql = _T("select now() as Systemtime;");
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecDataTime))
				{

				}
				if (vecDataTime.size() < 1)
				{
					CLogRecord::WriteRecordToFile(_T("ͬ��PC��-���ݲ���ʧ��!") + strSql);
					ret &= FALSE;
				}

				CStringArray** ppAryDataUpdate = nullptr;
				//0.ͬ��phone�˵� RFid �� State�ֶ�
				int RFidCount = 0;
				CList<CStringA> listUpdateSql; //������Ҫ���µ�sql���
				{
					strSql.Format(_T("SELECT count(*) FROM `work_task` where DeviceId in(%s) and SynchronState='0' and  CreateDate >= '%s' and CreateDate <= '%s';")
						, strDeviceID, strBeginTime, strCurTime);
					if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
					{
						if (RFidCount > 0)
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[RFidCount];
							for (int index = 0; index < RFidCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							ppAryDataUpdate = new CStringArray*[RFidCount];
							strSql.Format(_T("SELECT `Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId` ,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark FROM `work_task` \
					where DeviceId in(%s) and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';\
					"), strDeviceID, strBeginTime, strCurTime);
							if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
							{

								for (int index = 0; index < RFidCount; ++index)
								{
									if (ppAryData[index] == nullptr)
									{
										continue;
									}

									CStringA strTemp = ("Replace  INTO `work_task` (`Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark,SynchronTime) VALUES(");
									for (int indEnum = work_task_Id; indEnum < work_task_max - 1; ++indEnum)
									{

										if (indEnum == work_task_SynchronState)
										{
											strTemp += ("'1',");
										}
										else if (work_task_ExecutorTime == indEnum)
										{
											if (ppAryData[index]->GetAt(indEnum).IsEmpty())
											{
												strTemp += ("null,");
											}
											else
												strTemp += "'" + CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(indEnum)) + ("',");
										}
										else
											strTemp += "'" + CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(indEnum)) + ("',");
									}
									strTemp += "'" + CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_task_max - 1)) + "',";
									strTemp += "'" + CStringA(vecDataTime[0][0].c_str()) + "');";
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
					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + strSql);
						ret &= FALSE;
					}
				}
			

				//����mysql���ݿ�
				if (!UpdateMysqlDB(listUpdateSql))
				{
					ShowLog(_T("��phone����RFId�����״̬�������work_taskʧ��!"));
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));

					ret &= FALSE;
				}
				else
				{
					if (ppAryDataUpdate)
					{
						{
							//����phone ������ ״̬
							if (sqOne.QuickInsertData("update work_task set SynchronState='1' where Id = ?", ppAryDataUpdate, RFidCount, 1, CSQLite::sqlite3_bind))
							{

							}
							
						}
						for (int index = 0; index < RFidCount; ++index)
						{
							delete ppAryDataUpdate[index];
						}
						delete[] ppAryDataUpdate;
					}

				}

			}
			else if (tongbuDir == tongbu_to_phone) //��pcͬ�����ն�
			{

				//////////////////////////////////////////////////////////////////////////

				std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
																//1.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
				//1.ɾ������ ɾ��ͬ��״̬Ϊ2������
				{

					strSql.Format(_T("Select Id from work_task where AllId in(SELECT Id FROM `work_task` where Level='0' and   SynchronState='2')\
			or Id in(SELECT Id FROM `work_task` where Level = '0' and  SynchronState='2')\
			or DeviceId not in(select Id from device_info);"));
					vecData.clear();
					if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
					{
						int dataCount = vecData.size(), dataNum = 0;
						if (dataCount > 0)//�����ݿ���ͬ��
						{

							{
								CStringA strTemp;
								CStringArray** ppAryData = new CStringArray*[dataCount];
								for (int index = 0; index < dataCount; ++index)
								{
									ppAryData[index] = nullptr;
								}
								int index = 0;
								for each (std::vector<string> varVec in vecData)
								{

									ppAryData[index] = new CStringArray;
									ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_task_Id].c_str()));

									++index;
								}
								dataNum = ppAryData[0]->GetSize();
								//1.ɾ������
								BOOL retTemp = sqOne.QuickDeletData("delete from work_task where Id=?", ppAryData
									, dataCount, dataNum, CSQLite::sqlite3_bind);

								retTemp &= sqOne.QuickDeletData("delete from work_task where AllId=?", ppAryData
									, dataCount, dataNum, CSQLite::sqlite3_bind);

								//20180611-2.ɾ��������ؼ�¼ 
								retTemp &= sqOne.QuickDeletData("DELETE FROM work_record where WorkTaskId =?;", ppAryData
									, dataCount, dataNum, CSQLite::sqlite3_bind);

								retTemp &= sqOne.DeleteData("delete from work_record where WorkTaskId not in(select Id from work_task)");

								for (int index = 0; index < dataCount; ++index)
								{
									//2.ɾ�������ļ�¼��
									delete ppAryData[index];
								}

								delete[] ppAryData;

								if (!ret)
								{
									CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
									CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
									ret &= FALSE;
								}
							}

						}
					}
					else
					{

						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
						ret &= FALSE;
					}


					//2.2 ɾ��״̬Ϊ3������
					{
						strSql.Format(_T("Select Id from work_task where AllId in(SELECT Id FROM `work_task` where Level='0' and   SynchronState='3')\
			or Id in(SELECT Id FROM `work_task` where Level = '0' and  SynchronState='3')\
			or DeviceId not in(select Id from device_info);"));
						vecData.clear();
						if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
						{
							int dataCount = vecData.size(), dataNum = 0;
							if (dataCount > 0)//�����ݿ���ͬ��
							{
								//2.2.1 ɾ���ն����ݿ�����
								CList<CStringA> deleteList;
								{
									CStringA strTemp;
									CStringArray** ppAryData = new CStringArray*[dataCount];
									for (int index = 0; index < dataCount; ++index)
									{
										ppAryData[index] = nullptr;
									}
									int index = 0;
									for each (std::vector<string> varVec in vecData)
									{
										ppAryData[index] = new CStringArray;
										ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_task_Id].c_str()));

										++index;
									}
									dataNum = ppAryData[0]->GetSize();
									//1.ɾ������
									BOOL retTemp = sqOne.QuickDeletData("delete from work_task where Id=?", ppAryData
										, dataCount, dataNum, CSQLite::sqlite3_bind);
									retTemp &= sqOne.QuickDeletData("delete from work_task where AllId=?", ppAryData
										, dataCount, dataNum, CSQLite::sqlite3_bind);

									//20180611-2.ɾ��������ؼ�¼ 
									retTemp &= sqOne.QuickDeletData("DELETE FROM work_record where WorkTaskId =?;", ppAryData
										, dataCount, dataNum, CSQLite::sqlite3_bind);

									retTemp &= sqOne.DeleteData("delete from work_record where WorkTaskId not in(select Id from work_task)");

									CString strTempDelete;
									for (int index = 0; index < dataCount; ++index)
									{
										strTempDelete.Format(_T("delete from work_task where Id='%s';"), ppAryData[index]->GetAt(work_task_Id));
										deleteList.AddTail(CpublicFun::UnicodeToAsc(strTempDelete));

										strTempDelete.Format(_T("DELETE FROM work_record where WorkTaskId ='%s';"), ppAryData[index]->GetAt(work_task_Id));
										deleteList.AddTail(CpublicFun::UnicodeToAsc(strTempDelete));

										delete ppAryData[index];
									}

									delete[] ppAryData;

									if (!ret)
									{
										CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
										CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
										ret &= FALSE;
									}

								}

								//2.2.2 ɾ��pc������
								UpdateMysqlDB(deleteList);
							}
						}
						else
						{

							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
							ret &= FALSE;
						}

						

					}
					
				}

				//2.ͬ��������
				strSql.Format(_T("SELECT `Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark FROM `work_task` \
		where DeviceId in(%s) and SynchronState='0' and State<>'0' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
				//2.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
				CList<CStringA> updateMysqlData;
				vecData.clear();
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//�����ݿ���ͬ��
					{
						
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[dataCount];
							for (int index = 0; index < dataCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							int index = 0;

							for each (std::vector<string> varVec in vecData)
							{
								//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
								if (varVec[work_task_State] == "1")
								{
									strTemp.Format(("UPDATE work_task set SynchronState ='1' ,State='2' where Id='%s';")
										, (varVec[work_task_Id].c_str()));
								}
								else
									strTemp.Format(("UPDATE work_task set SynchronState ='1'  where Id='%s';")
										, (varVec[work_task_Id].c_str()));

								updateMysqlData.AddTail(strTemp.GetBuffer());
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
							CStringA strSql = "Replace  INTO `work_task` (`Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

							BOOL retTemp = sqOne.QuickInsertData(strSql
								, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}

							delete[] ppAryData;


							
							if (!retTemp)
							{
								CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
								ret &= FALSE;
							}
							else
							{
								POSITION posUpdate = updateMysqlData.GetHeadPosition();
								while (posUpdate != NULL)
								{
									m_updateMysqlData.AddTail(updateMysqlData.GetNext(posUpdate));
								}
							}

						}
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					ret &= FALSE;
				}

			}

			sqOne.CloseDataBase();
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			ret &= FALSE;
		}

		CLogRecord::WriteRecordToFile(_T("ͬ����[work_task]�������!"));
		return ret;
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
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_template(CStringArray const&  strAryAreaID, CString const& strDBPath, int tongbuDir)
{
	try
	{
		//work_template ֻ�ܴ�pcͬ�����ն�
		if (tongbuDir == tongbu_to_pc)
		{
			return TRUE;
		}
		//0.��ʼ����ɾ������
		if (!PathFileExists(strDBPath))
		{
			CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
			return FALSE;
		}

		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
		CString strSql;
		CString strDeviceID;
		/*
		ƴ���ַ���
		*/
		CString strTemp;
		for (int index = 0; index <strAryAreaID.GetSize(); ++index)
		{
			//��һ������
			if (index == 0)
			{
				strTemp.Format(_T("'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			else
			{
				strTemp.Format(_T(",'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}

		}

		BOOL retRes = TRUE;
		CSQLite sqOne;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
			//��ȡ����ID
			strSql.Format(_T("SELECT te.`Id`,te.`DeviceId`,te.`WorkTypeId`,te.ClassName,te.`Number`,te.`ProcessName`,te.`Results`,te.`SynchronState`,te.`CreateDate`,te.`CreateUserId` FROM `work_template` te,work_type tk \
		where (te.SynchronState ='0' or te.SynchronState ='1') and   tk.DeviceId in(%s) and tk.CreateDate >= '%s' and tk.CreateDate <= '%s' and tk.Id = te.WorkTypeId;\
		"), strDeviceID, strBeginTime, strCurTime);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			CList<CStringA> updateMysqlData;
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						for (int index = 0; index < dataCount; ++index)
						{
							ppAryData[index] = nullptr;
						}
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
							strTemp.Format(("UPDATE work_template set SynchronState ='1' where Id='%s';")
								, (varVec[work_template_Id].c_str()));
							updateMysqlData.AddTail(strTemp.GetBuffer());
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

						BOOL ret = sqOne.QuickInsertData(strSql
							, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}
						delete[] ppAryData;

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
							retRes &= FALSE;
						}
						else
						{
							POSITION posUpdate = updateMysqlData.GetHeadPosition();
							while (posUpdate != NULL)
							{
								m_updateMysqlData.AddTail(updateMysqlData.GetNext(posUpdate));
							}
						}
					}
					
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `Id` FROM  `work_template` where SynchronState='2'  and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strBeginTime, strCurTime);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
			
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						for (int index = 0; index < dataCount; ++index)
						{
							ppAryData[index] = nullptr;
						}
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[work_template_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sqOne.QuickDeletData("delete from work_template where Id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);

						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}
						delete[] ppAryData;



						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
							retRes &= FALSE;
						}
					}
					
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
			sqOne.CloseDataBase();
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);	
			retRes &= FALSE;
		}

		CLogRecord::WriteRecordToFile(_T("ͬ����[work_template]�������!"));
		return retRes;
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
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_type(CStringArray const&  strAryAreaID, CString const& strDBPath, int tongbuDir)
{
	try
	{
		//work_type ֻ�ܴ�pcͬ�����ն�
		if (tongbuDir == tongbu_to_pc)
		{
			return TRUE;
		}
		//0.��ʼ����ɾ������
		if (!PathFileExists(strDBPath))
		{
			CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
			return FALSE;
		}
		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
		CString strSql;

		CString strDeviceID ;
		/*
		ƴ���ַ���
		*/
		CString strTemp;
		for (int index =0; index <strAryAreaID.GetSize();++index)
		{
			//��һ������
			if (index == 0)
			{
				strTemp.Format(_T("'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			else
			{
				strTemp.Format(_T(",'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			
		}


		BOOL retRes = TRUE;
		CSQLite sq;
		if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
			//��ȡ����ID
		
			strSql.Format(_T("SELECT `Id`,`Name`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,DeviceId FROM `work_type`\
		 where SynchronState='0'  and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId in(%s);\
		"), strBeginTime, strCurTime,strDeviceID);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			CList<CStringA> updateMysqlData;
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						for (int index = 0; index < dataCount; ++index)
						{
							ppAryData[index] = nullptr;
						}
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
							strTemp.Format(("UPDATE work_type set SynchronState ='1' where Id='%s';")
								, (varVec[work_type_Id].c_str()));
							updateMysqlData.AddTail(strTemp.GetBuffer());
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


						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
							retRes &= FALSE;
						}
						else
						{
							POSITION posUpdate = updateMysqlData.GetHeadPosition();
							while (posUpdate != NULL)
							{
								m_updateMysqlData.AddTail(updateMysqlData.GetNext(posUpdate));
							}
						}
					}

				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `Id` FROM  `work_type` where SynchronState='2' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strBeginTime, strCurTime);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
				
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						for (int index = 0; index < dataCount; ++index)
						{
							ppAryData[index] = nullptr;
						}
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



						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							retRes &= FALSE;
						}
					}
					
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}
			sq.CloseDataBase();
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			retRes &= FALSE;
		}
		CLogRecord::WriteRecordToFile(_T("ͬ����[work_type]�������!"));
		return retRes;
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
BOOL  CWPD_MTP_dataDlg::Synchrodata_device_info(CStringArray const&  strAryAreaID,CString const& strDBPath,int tongbuDir,int tongbuFw)
{
	try
	{
		//��ѡ����û���豸 


		CString strDeviceID;
		/*
		ƴ���ַ���
		*/
		CString strTemp;
		for (int index = 0; index <strAryAreaID.GetSize(); ++index)
		{
			//��һ������
			if (index == 0)
			{
				strTemp.Format(_T("'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			else
			{
				strTemp.Format(_T(",'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}

		}


		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		CString strSql,strTempSql;
		//0.ͬ��phone�˵� RFid �ֶ�
		//���ն˵�pc
		BOOL retRes = TRUE;
		CSQLite sqOne;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			if (tongbu_to_pc == tongbuDir)
			{
				CList<CStringA> listUpdateSql; //������Ҫ���µ�sql���

				CStringArray** ppAryDataUpdate = nullptr;
				//0.ͬ��phone�˵� RFid �� State�ֶ�
				int RFidCount = 0;

				{

					BOOL ret = FALSE;
					if (tongbu_fw_areas == tongbuFw) //ͬ����Χ:����
					{
						strSql.Format(_T("SELECT count(*) FROM `device_info` where AreaId in(%s) and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
							, strDeviceID, strBeginTime, strCurTime);
						strTempSql.Format(_T("SELECT `Id`,`RFID` FROM `device_info` where AreaId in(%s) and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
							, strDeviceID, strBeginTime, strCurTime);
					}
					else if (tongbu_fw_devices == tongbuFw)//ͬ����Χ:�豸
					{
						strSql.Format(_T("SELECT count(*) FROM `device_info` where Id in(%s) and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
							, strDeviceID, strBeginTime, strCurTime);
						strTempSql.Format(_T("SELECT `Id`,`RFID` FROM `device_info` where Id in(%s) and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
							, strDeviceID, strBeginTime, strCurTime);
					}
					if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
					{
						if (RFidCount > 0)
						{
							CString strLog;
							strLog.Format(_T("ͬ������������Ϊ[%d];"), RFidCount);
							CLogRecord::WriteRecordToFile(strSql + strLog);
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[RFidCount];
							//��ʼ���ڴ����
							for (int index = 0; index < RFidCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							ppAryDataUpdate = new CStringArray*[RFidCount];

							

							if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strTempSql), ppAryData, RFidCount))
							{
								for (int index = 0; index < RFidCount; ++index)
								{
									if (ppAryData[index] == nullptr)
									{
										continue;
									}
									strTemp.Format(("UPDATE device_info set RFID ='%s' where Id='%s';")
										, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(device_info_RFID))
										, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(device_info_Id)));
									listUpdateSql.AddTail(strTemp);
									ppAryDataUpdate[index] = new CStringArray;

									ppAryDataUpdate[index]->Add(ppAryData[index]->GetAt(device_info_Id));
								}
								for (int index = 0; index < RFidCount; ++index)
								{
									delete ppAryData[index];
								}
							}
							delete[] ppAryData;
						}
					}
					
					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + strSql);
						retRes &= FALSE;
					}
				}


			//����mysql���ݿ�
			if (!UpdateMysqlDB(listUpdateSql))
			{
				ShowLog(_T("��phone����RFId�������ʧ��!"));
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}
			else
			{
				if (ppAryDataUpdate)
				{
					
					{
						//����phone ������ ״̬
						if (sqOne.QuickInsertData("update device_info set SynchronState='1' where Id = ?", ppAryDataUpdate, RFidCount, 1, CSQLite::sqlite3_bind))
						{

						}
						else
						{
							CLogRecord::WriteRecordToFile(_T("device_info ͬ��RFID ͬ�������ݲ���ʧ��!"));
							retRes &= FALSE;
						}
						
					}
					for (int index = 0; index < RFidCount; ++index)
					{
						delete ppAryDataUpdate[index];
					}
					delete[] ppAryDataUpdate;
				}

			}
			}
			else if (tongbu_to_phone == tongbuDir) //��pc���ն��豸  ͬ���豸��
			{
				//��ȡ����ID
				std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
				if (tongbu_fw_areas == tongbuFw) //ͬ����Χ:����
				{
					strSql.Format(_T("SELECT `Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,\
		`SynchronState`,`CreateDate`,`CreateUserId`,ParentId,IsBindRFID  FROM `device_info` where AreaId in(%s) and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
						, strDeviceID, strBeginTime, strCurTime);
				}
				else if (tongbu_fw_devices == tongbuFw)//ͬ����Χ:�豸
				{
					strSql.Format(_T("SELECT `Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,\
		`SynchronState`,`CreateDate`,`CreateUserId`,ParentId,IsBindRFID  FROM `device_info` where Id in(%s) and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
						, strDeviceID, strBeginTime, strCurTime);
				}

				//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
				CList<CStringA> updateMysqlData;
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//�����ݿ���ͬ��
					{
				
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[dataCount];
							//��ʼ���ڴ����
							for (int index = 0; index <dataCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
							int index = 0;

							for each (std::vector<string> varVec in vecData)
							{
								//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
								strTemp.Format(("UPDATE device_info set SynchronState ='1' where Id='%s';")
									, (varVec[device_info_Id].c_str()));
								updateMysqlData.AddTail(strTemp.GetBuffer());
								//m_areasDeviceID.AddTail(varVec[device_info_Id].c_str());
								ppAryData[index] = new CStringArray;
								for each (string var in varVec)
								{
									ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
								}
								ppAryData[index]->SetAt(device_info_SynchronState, _T("1"));
								CString temps = ppAryData[index]->GetAt(device_info_IsBindRFID);
								if (ppAryData[index]->GetAt(device_info_IsBindRFID) == _T("\x0"))
								{
									ppAryData[index]->SetAt(device_info_IsBindRFID, _T("0"));
								}
								if (ppAryData[index]->GetAt(device_info_IsBindRFID) == _T("\x1"))
								{
									ppAryData[index]->SetAt(device_info_IsBindRFID, _T("1"));
								}
								//CString tes = ppAryData[index]->GetAt(device_info_IsBindRFID);
								++index;
							}
							dataNum = ppAryData[0]->GetSize();
							//ִ�� û�оͲ���  �о͸���
							CStringA strSql = "Replace  INTO `device_info` (`Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,`SynchronState`,`CreateDate`,`CreateUserId`,ParentId,IsBindRFID ) VALUES(?,?,?,?,?,?,?,?,?,?,?,?);";
							//strSql = "insert into device_type(Id,Name) values (?,?);";
							BOOL ret = sqOne.QuickInsertData(strSql
								, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);

							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}

							delete[] ppAryData;


							

							if (!ret)
							{
								CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
								retRes &= FALSE;
							}
							else
							{
								POSITION posUpdate = updateMysqlData.GetHeadPosition();
								while (posUpdate != NULL)
								{
									m_updateMysqlData.AddTail(updateMysqlData.GetNext(posUpdate));
								}
							}
						}
					
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					retRes &= FALSE;
				}

				//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
				if (tongbu_fw_areas == tongbuFw) //ͬ����Χ:����
				{
					strSql.Format(_T("SELECT `Id` FROM `device_info` where AreaId in(%s) and SynchronState='2' \
	and CreateDate >= '%s' and CreateDate <= '%s' ;"), strDeviceID, strBeginTime, strCurTime);
				}
				else if (tongbu_fw_devices == tongbuFw)//ͬ����Χ:�豸
				{
					strSql.Format(_T("SELECT `Id` FROM `device_info` where  SynchronState='2' \
	and CreateDate >= '%s' and CreateDate <= '%s' ;"), strBeginTime, strCurTime);
// 					strSql.Format(_T("SELECT `Id` FROM `device_info` where Id='%s' and SynchronState='2' \
// 	and CreateDate >= '%s' and CreateDate <= '%s' ;"), strAreaID, strBeginTime, strCurTime);
				}
				vecData.clear();
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//�����ݿ���ͬ��
					{
						
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[dataCount];
							for (int index = 0; index < dataCount; ++index)
							{
								ppAryData[index] = nullptr;
							}
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
							BOOL ret = sqOne.QuickDeletData("delete from device_info where Id=?", ppAryData
								, dataCount, dataNum, CSQLite::sqlite3_bind);
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
							delete[] ppAryData;


							if (!ret)
							{
								CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
								CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
								retRes &= FALSE;
							}
						}
						
					}
				}
				else
				{

					CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					retRes &= FALSE;
				}
			}
			sqOne.CloseDataBase();
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			retRes &= FALSE;
		}
		
		CLogRecord::WriteRecordToFile(_T("ͬ����[device_info]�������!"));
		return retRes;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}

//areas
/*
* 20180620 ���ӹ���,ͬ���豸��ʱ��,ͬ���û���Ӧ������
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_areas(CString const& strDBPath,  int tongbuDir)
{
	try
	{
		//��ѡ����û���豸 

		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		CString strSql, strDeleteSql;
		//0.ͬ��phone�˵� RFid �ֶ�
		//���ն˵�pc
		BOOL retRes = TRUE;
		CSQLite sqOne;
		if (tongbu_to_phone == tongbuDir)
		{

			//1.����sql���
			

			POSITION pos = m_mapTreeIDToCtrl.GetStartPosition();
			CString strKey;
			HTREEITEM itemVale;
			BOOL firstFlag = TRUE;
			CString strTemp;
			CString strTempAll;
			while (pos)
			{
				m_mapTreeIDToCtrl.GetNextAssoc(pos, strKey, itemVale);
				if (!strKey.IsEmpty() && itemVale != nullptr) //��Чֵ
				{
					if (firstFlag)
					{
						strTemp.Format(_T("'%s'"), strKey);
						strTempAll += strTemp;
						firstFlag = FALSE;
					}
					else
					{
						//�Ժ������
						strTemp.Format(_T(",'%s'"), strKey);
						strTempAll += strTemp;
					}

				}
			
			}
			strSql.Format(_T("SELECT `areas`.`Id`,\
					`areas`.`Code`,\
					`areas`.`Name`,\
					`areas`.`ParentId`,\
					`areas`.`CreateDate`,\
					`areas`.`CreateUserId`,\
					`areas`.`Info`,\
					`areas`.`SynchronState`,\
					`areas`.`isDelete`\
					FROM `areas` where Id in(%s);"),strTempAll);
			if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
			{
				//��ȡ����ID
				std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������

				//1.��ʾ����Ҫͬ��������
				if(!firstFlag)
				{

					//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
					CList<CStringA> updateMysqlData;
					if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
					{
						int dataCount = vecData.size(), dataNum = 0;
						if (dataCount > 0)//�����ݿ���ͬ��
						{

							{
								CStringA strTemp;
								CStringArray** ppAryData = new CStringArray*[dataCount];
								//��ʼ���ڴ����
								for (int index = 0; index < dataCount; ++index)
								{
									ppAryData[index] = nullptr;
								}
								int index = 0;

								for each (std::vector<string> varVec in vecData)
								{

									ppAryData[index] = new CStringArray;
									for each (string var in varVec)
									{
										ppAryData[index]->Add(CpublicFun::AscToUnicode(var.c_str()));
									}
									ppAryData[index]->SetAt(areas_data_SynchronState, _T("1"));
									CString temps = ppAryData[index]->GetAt(areas_data_isDelete);
									if (ppAryData[index]->GetAt(areas_data_isDelete) == _T("\x0"))
									{
										ppAryData[index]->SetAt(areas_data_isDelete, _T("0"));
									}
									if (ppAryData[index]->GetAt(areas_data_isDelete) == _T("\x1"))
									{
										ppAryData[index]->SetAt(areas_data_isDelete, _T("1"));
									}
									
									++index;
								}
								dataNum = PHONE_AREAS_NUM;
								//ִ�� û�оͲ���  �о͸���
								CStringA strSql = "Replace  INTO `areas` (Id,Code,Name,ParentID,CreateDate,CreateUserId) VALUES(?,?,?,?,?,?);";
								//strSql = "insert into device_type(Id,Name) values (?,?);";
								BOOL ret = sqOne.QuickInsertData(strSql
									, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);

								for (int index = 0; index < dataCount; ++index)
								{
									delete ppAryData[index];
								}

								delete[] ppAryData;

								if (!ret)
								{
									CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
									retRes &= FALSE;
								}
							}

						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
						retRes &= FALSE;
						ShowLog(_T("ͬ���������ʧ��"));
					}
					
				}

				//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
				{
					
					strDeleteSql = _T("SELECT `areas`.`Id` FROM `areas` where SynchronState='2';");

					vecData.clear();
					if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strDeleteSql).GetBuffer(), vecData))
					{
						int dataCount = vecData.size(), dataNum = 0;
						if (dataCount > 0)//�����ݿ���ͬ��
						{

							{
								CStringA strTemp;
								CStringArray** ppAryData = new CStringArray*[dataCount];
								for (int index = 0; index < dataCount; ++index)
								{
									ppAryData[index] = nullptr;
								}
								int index = 0;
								for each (std::vector<string> varVec in vecData)
								{

									ppAryData[index] = new CStringArray;
									ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[device_type_Id].c_str()));

									++index;
								}
								dataNum = ppAryData[0]->GetSize();
								BOOL ret = sqOne.QuickDeletData("delete from areas where Id=?", ppAryData
									, dataCount, dataNum, CSQLite::sqlite3_bind);
								for (int index = 0; index < dataCount; ++index)
								{
									delete ppAryData[index];
								}
								delete[] ppAryData;

								if (!ret)
								{
									CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
									CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
									retRes &= FALSE;
								}
							}

						}
					}
					else
					{

						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
						retRes &= FALSE;
					}
				}

				sqOne.CloseDataBase();
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
				retRes &= FALSE;
			}
			
		}
		CLogRecord::WriteRecordToFile(_T("ͬ����[areas]�������!"));
		return retRes;
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
		//20180607  ȡ��ϵͳ�ĸ���
		return TRUE;



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
		FROM `device_type` where  SynchronState='0' or  SynchronState='1' and AreaId='%s' and  CreateDate >= '%s' and CreateDate <= '%s';\
		"), strAreaID, strBeginTime, strCurTime);
		//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
		CList<CStringA> updateMysqlData;
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
					for (int index = 0; index < dataCount; ++index)
					{
						ppAryData[index] = nullptr;
					}
					int index = 0;

					for each (std::vector<string> varVec in vecData)
					{
						//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�
						strTemp.Format(("UPDATE device_type set SynchronState ='1' where Id='%s';")
							, (varVec[device_type_Id].c_str()));
						updateMysqlData.AddTail(strTemp.GetBuffer());
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
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr()+ CpublicFun::AscToUnicode(strSql));
						return FALSE;
					}
					else
					{
						POSITION posUpdate = updateMysqlData.GetHeadPosition();
						while (posUpdate != NULL)
						{
							m_updateMysqlData.AddTail(updateMysqlData.GetNext(posUpdate));
						}
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
		strSql.Format(_T("SELECT `Id` FROM  `device_type` where SynchronState='2';"));
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
					for (int index = 0; index < dataCount; ++index)
					{
						ppAryData[index] = nullptr;
					}
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
					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + (strSql));
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


//��ʼ��ɾ�������ڴ�ṹ��
BOOL   CWPD_MTP_dataDlg::InitDeleteTreeStruct()
{
	BOOL retRes = TRUE;
	//1.������ṹ��
	retRes &= ClearDeleteTreeStruct(m_deleteTree);
	//2.�����µ��������ṹ��
	retRes &= CreateDeleteTreeStructAreas(m_deleteTree);

	//3.�����µ��豸���ṹ��
	
	//3.1 ��ȡ�豸�е� ���ڵ㲢������Ӧ����
	CString strKey, strValue;

	POSITION pos = m_mapTypeParentDelete.GetStartPosition();
	CMap<CString, LPCTSTR, deleteTree*, deleteTree*> mapDeviceIDToNode; //�豸ID��Ӧ�Ľڵ�

	while (pos)
	{
		m_mapTypeParentDelete.GetNextAssoc(pos, strKey, strValue);
		if (strValue == PARENTFLAG) //Ϊ���ڵ�
		{
			deleteTree* treeNodeTemp = nullptr;
			

			if (m_mapTypeToAreaDelete[strKey] == PARENTFLAG || m_mapTypeToAreaDelete[strKey] == _T(""))//����IDΪ0 ��ʾֱ���Ǹ��ڵ�
			{
				treeNodeTemp = new deleteTree;
				treeNodeTemp->m_strID = strKey;
				if (m_mapTypeStatusDelete[strKey] == DELETEFLAG
					|| m_mapTypeStatusDelete[strKey] == DELETEFLAG_WANT)//��־ɾ��
				{
					treeNodeTemp->m_deleteFlag = TRUE;
				}
				else
					treeNodeTemp->m_deleteFlag = FALSE;
				treeNodeTemp->m_strDeleteFlag = m_mapTypeStatusDelete[strKey];
				treeNodeTemp->m_typeFlag = delete_type_devices; //0���� 1�豸
				treeNodeTemp->m_parentID = strValue;
				treeNodeTemp->m_strName = m_mapTypeNameDelete[strKey];

				deleteTree* treeTemp = m_deleteTree->m_fistchild;
				if (treeTemp == nullptr) //1.��ʾû������ ,ֻ���豸
				{
					m_deleteTree->m_fistchild = treeNodeTemp;
					mapDeviceIDToNode[strKey] = treeNodeTemp; //���浱ǰ�豸ID��Ӧ�Ľڵ�
					continue; //������һ���豸
				}

				while (treeTemp)
				{
					//��ȡ�ֵܽڵ�
					if (treeTemp->m_nextsibling == nullptr) //2.�и�����,��ʾΪ���һ���ڵ���
					{
						treeTemp->m_nextsibling = treeNodeTemp;
						mapDeviceIDToNode[strKey] = treeNodeTemp; //���浱ǰ�豸ID��Ӧ�Ľڵ�
						break;//������һ���豸
					}
					else
					{
						treeTemp = treeTemp->m_nextsibling;
					}				
				}
			}
			else if(m_mapAreaIDToNode[m_mapTypeToAreaDelete[strKey]] != nullptr) //���ڸ��豸����Ľڵ�
			{
				treeNodeTemp = new deleteTree;
				treeNodeTemp->m_strID = strKey;
				if (m_mapTypeStatusDelete[strKey] == DELETEFLAG
					||m_mapTypeStatusDelete[strKey] == DELETEFLAG_WANT)//��־ɾ��
				{
					treeNodeTemp->m_deleteFlag = TRUE;
				}
				else
					treeNodeTemp->m_deleteFlag = FALSE;
				treeNodeTemp->m_strDeleteFlag = m_mapTypeStatusDelete[strKey];
				treeNodeTemp->m_typeFlag = delete_type_devices; //0���� 1�豸
				treeNodeTemp->m_parentID = strValue;
				treeNodeTemp->m_strName = m_mapTypeNameDelete[strKey];

				deleteTree* treeTemp = m_mapAreaIDToNode[m_mapTypeToAreaDelete[strKey]]->m_fistchild;
				if (treeTemp == nullptr) //û����Ӧ�ĵ�һ�����豸����ϵͳ
				{
					m_mapAreaIDToNode[m_mapTypeToAreaDelete[strKey]]->m_fistchild = treeNodeTemp;
					mapDeviceIDToNode[strKey] = treeNodeTemp; //���浱ǰ�豸ID��Ӧ�Ľڵ�
					continue; //������һ���豸
				}
				while (treeTemp)
				{
					//��ȡ�ֵܽڵ�
					if (treeTemp->m_nextsibling == nullptr) //2.�и�����,��ʾΪ���һ���ڵ���
					{
						treeTemp->m_nextsibling = treeNodeTemp;
						mapDeviceIDToNode[strKey] = treeNodeTemp; //���浱ǰ�豸ID��Ӧ�Ľڵ�
						break;//������һ���豸
					}
					else
					{
						treeTemp = treeTemp->m_nextsibling;
					}
				}
			}
		}
	}
	
	//3.2 �������������ڵ� ������Ӧ���ӽڵ�
	deleteTree* treeValue;
	pos = mapDeviceIDToNode.GetStartPosition();

	while (pos)
	{
		mapDeviceIDToNode.GetNextAssoc(pos, strKey, treeValue);
		retRes &= CreateDeleteTreeStructDevices(treeValue);
	}
	
	return retRes;
}

//�ݹ�ɾ�����ṹ
BOOL   CWPD_MTP_dataDlg::ClearDeleteTreeStruct(deleteTree* &treeNode)
{
	//1.�ڵ�Ϊ��
	if (!treeNode)
	{
		return TRUE;
	}
	//2.�����ӽڵ�
	if (treeNode->m_fistchild)
	{
		//2.1�ݹ�ɾ����Ӧ���ӽڵ�
		ClearDeleteTreeStruct(treeNode->m_fistchild);
	}
	//3.�����ֵܽڵ�
	if (treeNode->m_nextsibling)
	{
		//3.1�ݹ�ɾ����Ӧ���ӽڵ�
		ClearDeleteTreeStruct(treeNode->m_nextsibling);
	}
	//4.ɾ���ýڵ�
	delete treeNode;
	treeNode = nullptr;
	return TRUE;
}

//��������
BOOL  CWPD_MTP_dataDlg::CreateDeleteTreeStructAreasC(deleteTree* &treeNode)
{

	if (treeNode == nullptr) //�ڵ�Ϊ��,�������ӽڵ�
	{
		return TRUE;
	}
	
	deleteTree* tempSib = nullptr; //���һ���ֵܽڵ�
	
	CString strKey, strValue;

		POSITION pos = m_mapAreaParentDelete.GetStartPosition();
		
		while (pos)
		{
			deleteTree* treeNodeTemp = nullptr;
			m_mapAreaParentDelete.GetNextAssoc(pos, strKey, strValue);
			if (strValue == treeNode->m_strID) //Ϊ���׽ڵ�
			{
				treeNodeTemp = new deleteTree;
				treeNodeTemp->m_strID = strKey;
				if (m_mapAreaStatusDelete[strKey] == DELETEFLAG
					|| m_mapAreaStatusDelete[strKey] == DELETEFLAG_WANT)//��־ɾ��
				{
					treeNodeTemp->m_deleteFlag = TRUE;
				}
				else
					treeNodeTemp->m_deleteFlag = FALSE;
				treeNodeTemp->m_strDeleteFlag = m_mapAreaStatusDelete[strKey];
				treeNodeTemp->m_typeFlag = delete_type_areas; //0���� 1�豸
				treeNodeTemp->m_parentID = strValue;
				treeNodeTemp->m_strName = m_mapAreaNameDelete[strKey];
				//��һ����Ϊ���ڵ�
				if (treeNode->m_fistchild == nullptr)
				{
					treeNode->m_fistchild = treeNodeTemp;
				}
				else //��Ϊ�ֵܽڵ����
				{
					tempSib->m_nextsibling = treeNodeTemp;
				}
				tempSib = treeNodeTemp;
				CreateDeleteTreeStructAreasC(treeNodeTemp);
				m_mapAreaIDToNode[strKey] = treeNodeTemp;
			}
		}
	return TRUE;
}

//�ݹ鴴���������ṹ
BOOL   CWPD_MTP_dataDlg::CreateDeleteTreeStructAreas(deleteTree* &treeNode)
{

	//0.�����м�ṹ
	m_mapAreaIDToNode.RemoveAll();
	//1.���ڵ��ʼ���ýڵ�
	if (!treeNode)
	{
		treeNode = new deleteTree;
		//1.1 20180618 ����һ�� �û�Ȩ��. ֻ������ǰ��½�û��� �豸�� 
		//�û�����ǿ�
		if (!m_userArea.IsEmpty())
		{
			CString strKey, strValue;

			POSITION pos = m_mapAreaParentDelete.GetStartPosition();

			while (pos)
			{
				
				m_mapAreaParentDelete.GetNextAssoc(pos, strKey, strValue);
				if (strKey == m_userArea) //Ϊ���׽ڵ�
				{
					
					treeNode->m_strID = strKey;
					if (m_mapAreaStatusDelete[strKey] == DELETEFLAG
						|| m_mapAreaStatusDelete[strKey] == DELETEFLAG_WANT)//��־ɾ��
					{
						treeNode->m_deleteFlag = TRUE;
					}
					else
						treeNode->m_deleteFlag = FALSE;
					treeNode->m_strDeleteFlag = m_mapAreaStatusDelete[strKey];
					treeNode->m_typeFlag = delete_type_areas; //0���� 1�豸
					treeNode->m_parentID = strValue;
					treeNode->m_strName = m_mapAreaNameDelete[strKey];

					m_mapAreaIDToNode[strKey] = treeNode;
				}
			}
		}
	}
	
	//2.��ȡ���ڵ�Ķ��ӽڵ�
	CreateDeleteTreeStructAreasC(treeNode);

	return TRUE;
}

//�ݹ鴴���豸���ṹ
BOOL  CWPD_MTP_dataDlg::CreateDeleteTreeStructDevices(deleteTree* &treeNode)
{

	if (treeNode == nullptr) //�ڵ�Ϊ��,�������ӽڵ�
	{
		return TRUE;
	}

	deleteTree* tempSib = nullptr; //���һ���ֵܽڵ�

	CString strKey, strValue;

	POSITION pos = m_mapTypeParentDelete.GetStartPosition();

	while (pos)
	{
		
		m_mapTypeParentDelete.GetNextAssoc(pos, strKey, strValue);
		
		if (strValue == treeNode->m_strID) //Ϊ���׽ڵ�
		{
			deleteTree* treeNodeTemp = nullptr;
			treeNodeTemp = new deleteTree;
			treeNodeTemp->m_strID = strKey;
			if (m_mapTypeStatusDelete[strKey] == DELETEFLAG
				||m_mapTypeStatusDelete[strKey] == DELETEFLAG_WANT)//��־ɾ��
			{
				treeNodeTemp->m_deleteFlag = TRUE;
			}
			else
				treeNodeTemp->m_deleteFlag = FALSE;
			treeNodeTemp->m_strDeleteFlag = m_mapTypeStatusDelete[strKey];
			treeNodeTemp->m_typeFlag = delete_type_devices; //0���� 1�豸
			treeNodeTemp->m_parentID = strValue;
			treeNodeTemp->m_strName = m_mapTypeNameDelete[strKey];
			
			//��һ����Ϊ���ڵ�
			if (treeNode->m_fistchild == nullptr)
			{
				treeNode->m_fistchild = treeNodeTemp;
			}
			else //��Ϊ�ֵܽڵ����
			{
				tempSib->m_nextsibling = treeNodeTemp;
			}
			tempSib = treeNodeTemp;
			CreateDeleteTreeStructDevices(treeNodeTemp);
		}
		
	}
	return TRUE;
}


//�ݹ��ȡ��Ҫɾ������ӽڵ� ���ӽڵ���ֵܽڵ�
BOOL  CWPD_MTP_dataDlg::DeleteFindID(deleteTree* & treeNode
	, CMap<CString, LPCTSTR, CString, LPCTSTR>& strAryAreas
	, CMap<CString, LPCTSTR, CString, LPCTSTR>& strAryDevices)
{
	//�ݹ鷵��
	if (treeNode == nullptr)
		return TRUE;
	
	//2.1���� ɾ��
	if (treeNode->m_typeFlag == delete_type_areas)
	{
		strAryAreas.SetAt(treeNode->m_strID, treeNode->m_strID);
	}
	//2.2�豸
	if (treeNode->m_typeFlag == delete_type_devices)
	{
		strAryDevices.SetAt(treeNode->m_strID, treeNode->m_strID);
	}
	//������ȡ�ӽڵ㼰�ӽڵ���ֵܽڵ�
	DeleteFindID(treeNode->m_fistchild, strAryAreas, strAryDevices);
	//�ݹ��ֵܽڵ�
	DeleteFindID(treeNode->m_nextsibling, strAryAreas, strAryDevices);
	return TRUE;
}

//�ݹ��ȡ ��Ҫɾ����������豸 �������ǵ���������豸
BOOL  CWPD_MTP_dataDlg::DeleteAreaAndDeviceFindID(deleteTree* & treeNode
	, CMap<CString, LPCTSTR, CString, LPCTSTR>& strAryAreas, CMap<CString, LPCTSTR, CString, LPCTSTR>& strAryDevices
	, CMap<CString, LPCTSTR, CString, LPCTSTR>& strAryAreasWant, CMap<CString, LPCTSTR, CString, LPCTSTR>& strAryDevicesWant)
{
	//1.��������
	if (treeNode == nullptr)
	{
		return TRUE;
	}
	//2.��ʾ ��Ҫɾ������������豸 ��ȡ�����������豸 �������ӽڵ���ֵܽڵ�
	if (treeNode->m_deleteFlag)
	{
		//2.0.1 ��ɾ
		if (treeNode->m_strDeleteFlag == DELETEFLAG)
		{
			//2.1���� ɾ��
			if (treeNode->m_typeFlag == delete_type_areas)
			{
				strAryAreas.SetAt(treeNode->m_strID, treeNode->m_strID);
			}
			//2.2�豸
			if (treeNode->m_typeFlag == delete_type_devices)
			{
				strAryDevices.SetAt(treeNode->m_strID, treeNode->m_strID);
			}
			//������ȡ�ӽڵ㼰�ӽڵ���ֵܽڵ�
			DeleteFindID(treeNode->m_fistchild, strAryAreas, strAryDevices);
		}
		//2.0.2 ��ɾ
		if (treeNode->m_strDeleteFlag == DELETEFLAG_WANT)
		{
			//2.1���� ɾ��
			if (treeNode->m_typeFlag == delete_type_areas)
			{
				strAryAreasWant.SetAt(treeNode->m_strID, treeNode->m_strID);
			}
			//2.2�豸
			if (treeNode->m_typeFlag == delete_type_devices)
			{
				strAryDevicesWant.SetAt(treeNode->m_strID, treeNode->m_strID);
			}
			//������ȡ�ӽڵ㼰�ӽڵ���ֵܽڵ�
			DeleteFindID(treeNode->m_fistchild, strAryAreasWant, strAryDevicesWant);
		}
	}
	//3.�ݹ��ӽڵ��ȡ��Ҫɾ������
	DeleteAreaAndDeviceFindID(treeNode->m_fistchild, strAryAreas, strAryDevices, strAryAreasWant, strAryDevicesWant);
	//4.�ݹ��ֵܽڵ��ȡ��Ҫɾ������
	DeleteAreaAndDeviceFindID(treeNode->m_nextsibling, strAryAreas, strAryDevices, strAryAreasWant, strAryDevicesWant);
	
}
#define FUNMAPTOSTRARY(X,Y) {\
CString strKey, strValue;\
POSITION posParent = X.GetStartPosition();\
while (posParent)\
{\
	X.GetNextAssoc(posParent, strKey, strValue);\
	if (!strKey.IsEmpty())\
	{\
		Y.Add(strKey);\
	}\
}\
}

//ɾ����Ӧ����������豸  SynchronState Ϊ3������
BOOL  CWPD_MTP_dataDlg::DeleteAreaAndDevice(CString const& strDBPath)
{

	try
	{
		//1.��ȡ�ڴ�������Ҫɾ�������ݽṹ ��������豸�������鱣��
		//û��������豸������
		if (m_deleteTree == nullptr)
		{
			return TRUE;
		}
		CMap<CString,LPCTSTR,CString,LPCTSTR> strAryAreasMap, strAryDevicesMap, strAryAreasWantMap, strAryDevicesWantMap;
		CStringArray strAryAreas, strAryDevices, strAryAreasWant, strAryDevicesWant;
		DeleteAreaAndDeviceFindID(m_deleteTree, strAryAreasMap, strAryDevicesMap, strAryAreasWantMap, strAryDevicesWantMap);

		//ת��map �� �ַ�������
		FUNMAPTOSTRARY(strAryAreasMap, strAryAreas)
		FUNMAPTOSTRARY(strAryDevicesMap, strAryDevices)
		FUNMAPTOSTRARY(strAryAreasWantMap, strAryAreasWant)
		FUNMAPTOSTRARY(strAryDevicesWantMap, strAryDevicesWant)

		BOOL retRes = TRUE;
		//2.��ʼ����ɾ������
		if (!PathFileExists(strDBPath))
		{
			CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
			return FALSE;
		}

		CSQLite sq;
		if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			//2.1 ɾ���ն����ݿ� ����Ҫɾ���豸
			if (!strAryDevices.IsEmpty() || !strAryDevicesWant.IsEmpty())
			{

				CString strSql;
				BOOL retRes = TRUE;

				{
					int dataNumReal = strAryDevices.GetSize();
					int dataNumWant = strAryDevicesWant.GetSize();
					int dataNum = dataNumReal + dataNumWant;
					int dataCount = 1;
					CStringArray** ppAryData = new CStringArray*[dataNum];
					for (int index = 0; index < dataNum; ++index)
					{
						ppAryData[index] = nullptr;
					}
					for (int index = 0; index < dataNumReal; ++index)
					{
						ppAryData[index] = new CStringArray;
						ppAryData[index]->Add(strAryDevices.GetAt(index));
					}
					for (int index = 0; index < dataNumWant; ++index)
					{
						ppAryData[index + dataNumReal] = new CStringArray;
						ppAryData[index + dataNumReal]->Add(strAryDevicesWant.GetAt(index));
					}

					BOOL ret = TRUE;
					ret &= sq.QuickDeletData("delete from device_info where Id=?", ppAryData
						, dataNum, dataCount, CSQLite::sqlite3_bind);


					ret &= sq.DeleteData("DELETE FROM work_type WHERE DeviceId NOT in (SELECT id FROM device_info);\
					DELETE FROM work_template WHERE WorkTypeId NOT in(SELECT id FROM work_type);\
				DELETE FROM work_task WHERE DeviceId NOT in(SELECT id FROM device_info);\
				DELETE FROM work_record WHERE DeviceId NOT in(SELECT id from device_info);");

					//ɾ���������ݳɹ�
					if (ret)
					{
						CList<CStringA> strAryDelete;
						CStringA strTemp;
						//����
						for (int index = 0; index < strAryDevices.GetSize(); ++index)
						{
							strTemp.Format(("delete from device_info where Id='%s';")
								, (CpublicFun::UnicodeToAsc(strAryDevices.GetAt(index))));
							strAryDelete.AddTail(strTemp.GetBuffer());
						}

						ret &= UpdateMysqlDB(strAryDelete);

						strTemp.Format(("DELETE FROM work_type WHERE DeviceId NOT in (SELECT id FROM device_info);\
					DELETE FROM work_template WHERE WorkTypeId NOT in(SELECT id FROM work_type);\
				DELETE FROM work_task WHERE DeviceId NOT in(SELECT id FROM device_info);\
				DELETE FROM work_record WHERE DeviceId NOT in(SELECT id from device_info);"));

						ret &= CMyDataBase::GetInstance()->Query(strTemp.GetBuffer());

					}

					retRes = ret;
					for (int index = 0; index < dataNum; ++index)
					{
						delete ppAryData[index];
					}
					delete[] ppAryData;


				}

			}

			//2.2 ��� ������Ϣ �������Ӧ���û���Ϣ
			if (!strAryAreas.IsEmpty() || !strAryAreasWant.IsEmpty())
			{
				int ret = TRUE;
				int dataNumRealAreas = strAryAreas.GetSize();
				int dataNumWantAreas = strAryAreasWant.GetSize();
				int dataNumAreas = dataNumRealAreas + dataNumWantAreas;
				int dataCountAreas = 1;
				CStringArray** ppAryDataAreas = new CStringArray*[dataNumAreas];
				for (int index = 0; index < dataNumAreas; ++index)
				{
					ppAryDataAreas[index] = nullptr;
				}
				for (int index = 0; index < dataNumRealAreas; ++index)
				{
					ppAryDataAreas[index] = new CStringArray;
					ppAryDataAreas[index]->Add(strAryAreas.GetAt(index));
				}
				for (int index = 0; index < dataNumWantAreas; ++index)
				{
					ppAryDataAreas[index + dataNumRealAreas] = new CStringArray;
					ppAryDataAreas[index + dataNumRealAreas]->Add(strAryAreasWant.GetAt(index));
				}


				ret &= sq.QuickDeletData("delete from areas where Id=?", ppAryDataAreas
					, dataNumAreas, dataCountAreas, CSQLite::sqlite3_bind);

				//20180621 ����ɾ�������ʱ�� ɾ����Ӧ���û�
				ret &= sq.QuickDeletData("delete from user_table where data_areas=?", ppAryDataAreas
					, dataNumAreas, dataCountAreas, CSQLite::sqlite3_bind);

				//ɾ���������ݳɹ�
				if (ret)
				{
					CStringA strTemp;
					CList<CStringA> strAryDelete;
					//��ɾʱ,ɾ����Ӧ�������  ��������ص� �û���
					for (int index = 0; index < strAryAreas.GetSize(); ++index)
					{
						//ɾ�����
						strTemp.Format(("delete from areas where Id='%s';")
							, (CpublicFun::UnicodeToAsc(strAryAreas.GetAt(index))));
						strAryDelete.AddTail(strTemp.GetBuffer());
						//ɾ�û���
						strTemp.Format("delete from sys_user where data_areas='%s';"
							, (CpublicFun::UnicodeToAsc(strAryAreas.GetAt(index))));
						strAryDelete.AddTail(strTemp.GetBuffer());
					}
					ret &= UpdateMysqlDB(strAryDelete);
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("�������:ɾ���������ݴ���!"));
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				}

				for (int index = 0; index < dataNumAreas; ++index)
				{
					delete ppAryDataAreas[index];
				}
				delete[] ppAryDataAreas;
				retRes &= ret;
			}

			//2.3 �û�����ɾ
			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
			CString strSql;
			strSql.Format(_T("SELECT `id` FROM  `sys_user` where SynchronState='2' or SynchronState='3';"));
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					for (int index = 0; index < dataCount; ++index)
					{
						ppAryData[index] = nullptr;
					}
					int index = 0;
					for each (std::vector<string> varVec in vecData)
					{
						ppAryData[index] = new CStringArray;
						ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[sys_user_Id].c_str()));
						++index;
					}
					dataNum = ppAryData[0]->GetSize();
					BOOL ret = sq.QuickDeletData("delete from user_table where id=?", ppAryData
						, dataCount, dataNum, CSQLite::sqlite3_bind);
					for (int index = 0; index < dataCount; ++index)
					{
						delete ppAryData[index];
					}
					delete[] ppAryData;

					if (!ret)
					{
						CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
						CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
						retRes &= FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("��ɾ�û�����!"));
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}


			//ɾ�� Զ�˵� ��ɾ����
			CStringA strSqlA;
			strSqlA.Format(("delete FROM  `sys_user` where SynchronState='3';"));
			if (CMyDataBase::GetInstance()->OneQuery(strSqlA.GetBuffer()))
			{
			}
			else
			{
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(
					CMyDataBase::GetInstance()->GetErrorInfo() + strSqlA));
				retRes &= FALSE;
			}

			//�ر����ݿ�
			sq.CloseDataBase();
		}
		else
		{
			
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			retRes &= FALSE;
		}

		CLogRecord::WriteRecordToFile(_T("ɾ���������!"));
		return retRes;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}


}

//�������豸����ID
void CWPD_MTP_dataDlg::FindChildTypeID(CStringArray & strAry, HTREEITEM & item)
{
	//ѡ����豸���� �Լ� ���е� ���豸���� �������豸
	///
	if (item == nullptr)
	{
		return;
	}
	if (m_mapTreeCtrToIDType[item] != _T(""))
	{
		strAry.Add(m_mapTreeCtrToIDType[item]);
	}
	
	HTREEITEM hCurItem = m_tree_areas.GetChildItem(item);

	HTREEITEM hNextItem;
	while (hCurItem)
	{
		hNextItem = hCurItem;
		FindChildTypeID(strAry,hNextItem);
		hCurItem = m_tree_areas.GetNextSiblingItem(hCurItem);
	}
	
}

//ѡ�������������ID
void CWPD_MTP_dataDlg::FindChildID(CStringArray & strAry, HTREEITEM & item)
{
	//ѡ����豸���� �Լ� ���е� ���豸���� �������豸
	///
	if (item == nullptr)
	{
		return;
	}
	if (m_mapTreeCtrToID[item] != _T("")) //��Ч����
	{
		strAry.Add(m_mapTreeCtrToID[item]);
	}
	HTREEITEM hCurItem = m_tree_areas.GetChildItem(item);

	HTREEITEM hNextItem;
	while (hCurItem)
	{
		hNextItem = hCurItem;
		FindChildID(strAry, hNextItem);
		hCurItem = m_tree_areas.GetNextSiblingItem(hCurItem);
	}

}

//��ʼ����������豸����
//strTypeId :strAreaID�������µ�   ϵͳ����
BOOL CWPD_MTP_dataDlg::Init_area_devices(
	 CStringArray const& strAryAreaID, CStringArray & strAryDeviceID)
{
	try
	{
		strAryDeviceID.RemoveAll();


		//�޶�һ��ʱ�䷶Χ
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
// 		CStringArray strTypeId; //������ϵͳID
// 		//��ȡ��ѡ��ϵͳ �Լ���ϵͳ ������ID
// 		FindChildTypeID(strTypeId,m_treeCtrl_curItemType);
		for(int indexAreaID = 0; indexAreaID < strAryAreaID.GetSize(); ++indexAreaID)
		{

			std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
			CString strSql;
			strSql.Format(_T("SELECT `Id` FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s';")
				, strAryAreaID[indexAreaID], strBeginTime, strCurTime);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					for each (std::vector<string> varVec in vecData)
					{
						//2.���� ���� �ַ��� ����������,�ȵ� ͬ����� �� д��mysql���ݿ�

						strAryDeviceID.Add(CpublicFun::AscToUnicode(varVec[0].c_str()));

					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
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
BOOL  CWPD_MTP_dataDlg::Synchrodata_sys_user(CStringArray const&  strAryAreaID, CString const& strDBPath)
{
	try
	{
	
		//0.��ʼ����ɾ������
		if (!PathFileExists(strDBPath))
		{
			CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
			return FALSE;
		}

		CString strDeviceID;
		/*
		ƴ���ַ���
		*/
		CString strTemp;
		for (int index = 0; index <strAryAreaID.GetSize(); ++index)
		{
			//��һ������
			if (index == 0)
			{
				strTemp.Format(_T("'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}
			else
			{
				strTemp.Format(_T(",'%s'"), strAryAreaID.GetAt(index));
				strDeviceID += strTemp;
			}

		}



		BOOL retRes = TRUE;
		CSQLite sqOne;
		if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
		{
			std::vector<std::vector<std::string> > vecData; //mysql ���ݿ������
			CString strSql;

			POSITION posDeviceID = m_areasDeviceID.GetHeadPosition();
			
			int sqlNum = 0;

			strSql.Format(_T("SELECT u.id,u.user_name,u.real_name,u.password,r.role_name,a.Name, u.SynchronState ,u.pwd_key,u.role_id ,u.data_areas FROM `sys_user` u , sys_role r, areas a\
		where  (u.SynchronState='0' or u.SynchronState='1') and u.data_areas in(%s) and u.data_areas = a.Id and u.role_id = r.id;\
		"), strDeviceID);
			//1.ͬ��ѡ������  δͬ��������  ����ͬ��״̬��Ϊ ��ͬ��
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{

					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						for (int index = 0; index < dataCount; ++index)
						{
							ppAryData[index] = nullptr;
						}
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
						CStringA strSql = "Replace  INTO `user_table` (`Id`,`LoginName`,`Name`,`Password`,`Role`,`AreaName`,`SynchronState`,pwd_key,RoleId,data_areas) VALUES(?,?,?,?,?,?,?,?,?,?);";

						BOOL ret = sqOne.QuickInsertData(strSql
							, ppAryData, dataCount, dataNum, CSQLite::sqlite3_bind);
						for (int index = 0; index < dataCount; ++index)
						{
							delete ppAryData[index];
						}
						delete[] ppAryData;


						

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
							retRes &= FALSE;
						}
					}

				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}

			//2.ͬ��״̬Ϊ ��ɾ������ɾ��phone�� 
			strSql.Format(_T("SELECT `id` FROM  `sys_user` where SynchronState='2' and data_areas in(%s);\
		"), strDeviceID);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//�����ݿ���ͬ��
				{
					
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						for (int index = 0; index < dataCount; ++index)
						{
							ppAryData[index] = nullptr;
						}
						int index = 0;
						for each (std::vector<string> varVec in vecData)
						{

							ppAryData[index] = new CStringArray;
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[sys_user_Id].c_str()));

							++index;
						}
						dataNum = ppAryData[0]->GetSize();
						BOOL ret = sqOne.QuickDeletData("delete from user_table where id=?", ppAryData
							, dataCount, dataNum, CSQLite::sqlite3_bind);
						if (ret)
						{
							for (int index = 0; index < dataCount; ++index)
							{
								delete ppAryData[index];
							}
						}
						delete[] ppAryData;

						if (!ret)
						{
							CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
							CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
							retRes &= FALSE;
						}
					}
					
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("ɾ������ʧ��!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				retRes &= FALSE;
			}


			sqOne.CloseDataBase();
		}
		else
		{
			
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			return FALSE;
		}
		
		CLogRecord::WriteRecordToFile(_T("ͬ����[sys_user]�������!"));
		return retRes;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("ִ���쳣,���������Ϊ[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}

}

//ɾ����ʷ���� ����ѡ���ʱ�䷶Χ ɾ����Ӧ�� ��¼����ʷ
/*
ɾ������:
work_type �ֶ�LevelΪ0�ı�ʾ ������ ��State�ֶ�Ϊ3 ��ʾ����� ����ɾ��
������(AllIdΪ������ID)  ��¼��work_record WorkTaskId�ֶ�Ϊwork_type�����е�Id
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_clear_record(CString const& strDBPath, CString const& strBeginT, CString const& strEndt)
{
	//0.��ʼ����ɾ������
	if (!PathFileExists(strDBPath))
	{
		CLogRecord::WriteRecordToFile(_T("���ݿ��ļ�������:") + strDBPath);
		return FALSE;
	}
	CSQLite sqOne;
	BOOL ret = TRUE;
	if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
	{
		BOOL ret = FALSE;
		CString strSql;
		strSql.Format(_T("Select count(*) from work_task where AllId in(SELECT Id FROM `work_task` where Level='0' and (State = '3' or State = '6')  and  CreateDate >= '%s' and CreateDate <= '%s')\
			or Id in(SELECT Id FROM `work_task` where Level = '0' and (State = '3' or State = '6')  and  CreateDate >= '%s' and CreateDate <= '%s'); ")
			, strBeginT, strEndt, strBeginT, strEndt);
		int clearCount = 0;
		if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), clearCount))
		{
			if (clearCount > 0)
			{
				CStringA strTemp;
				CStringArray** ppAryData = new CStringArray*[clearCount];
				for (int index = 0; index < clearCount; ++index)
				{
					ppAryData[index] = nullptr;
				}

				//1.���ʱ����ڿ���ɾ������������
				strSql.Format(_T("Select Id from work_task where AllId in(SELECT Id FROM `work_task` where Level='0' and (State = '3' or State = '6')  and  CreateDate >= '%s' and CreateDate <= '%s')\
			or Id in(SELECT Id FROM `work_task` where Level = '0' and (State = '3' or State = '6')  and  CreateDate >= '%s' and CreateDate <= '%s'); ")
					, strBeginT, strEndt, strBeginT, strEndt);
				
				if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, clearCount)
					&& clearCount>0)
				{
					int dataNum = ppAryData[0]->GetSize();//ֻ��Idһ��
					//1.ɾ������
					 ret &= sqOne.QuickDeletData("delete from work_task where Id=?", ppAryData
						, clearCount, dataNum, CSQLite::sqlite3_bind);
					//20180611-ɾ��������ؼ�¼ 
					ret &= sqOne.QuickDeletData("DELETE FROM work_record where WorkTaskId =?;", ppAryData
						, clearCount, dataNum, CSQLite::sqlite3_bind);

					for (int index = 0; index < clearCount; ++index)
					{
						CString strTemp = ppAryData[index]->GetAt(0);
						delete ppAryData[index];
					}
				}
				delete[] ppAryData;
			}

			sqOne.CloseDataBase();
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("ͬ�������ݲ���ʧ��!") + strSql);
			CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
			return FALSE;
		}
	}
	else
	{
		
		CLogRecord::WriteRecordToFile(_T("���м����ݿ�ʧ��!") + strDBPath);
		return FALSE;
	}

	return ret;
}

void  GetArrayItems(CStringArray const& strAryItem, CStringArray* &pStrAry,int &retSize) 
{
	//1.�����������
	int strArySize = strAryItem.GetSize();
	int maxIndex = strArySize / MAXARRAYSIZE;
	
	int maxTemp = strArySize%MAXARRAYSIZE == 0 ? 0 : 1;
	maxIndex += maxTemp;
	if (maxIndex == 0)
	{
		return;
	}
	retSize = maxIndex;
	pStrAry = new CStringArray[maxIndex];

	CString strTemp;
	int curIndex = 0;
	int curAllIndex = 0;
	for (curAllIndex =0; curAllIndex <maxIndex;++curAllIndex)
	{
	
		int curIndexTemp = curIndex;
		for (int ind = curIndexTemp
			; ind < curIndexTemp + MAXARRAYSIZE && ind < strAryItem.GetSize()
			; ++ind, ++curIndex)
		{
			strTemp = strAryItem.GetAt(ind);
			if (strTemp.IsEmpty())
			{
				break;
			}
			pStrAry[curAllIndex].Add(strTemp);
		}
	}
}

void  ReleasArrayItems(CStringArray* &pStrAry, int &retSize)
{
	delete[] pStrAry;
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

			//0.ɾ���ն˺�pc����Ҫɾ���� ���� �豸��������� ����ͼ�¼
			DeleteAreaAndDevice(gFilePath);

			//ѡ��ĵ�ǰ ����ID �����豸ID
			CString strAreaID;
			BOOL ret = TRUE;
			/*
			20180530
			�޸�:����ѡ�� ����ϵͳѡ��
			1.����ѡ�� ͬ�� ���������������� �µ�����ϵͳ
			2.ϵͳѡ�� ͬ����Ӧϵͳ�µ�����
			*/

			//20180607
			//ͬ��������ѡ�� ͬ������������:ͬ���û�  ͬ���豸  ͬ������
			//ͬ������:���ն�ͬ����pc�� �� ��pc��ͬ�����ն�
			if (radio_user == m_radioChoose) //ͬ���û�
			{
				//�û�ֻ�� ��pc�� �ն�ͬ��
				if (m_mapTreeCtrToIDType[m_treeCtrl_curItem] != _T("")) //2.ϵͳѡ��
				{
					//�û�ֻ����������
					ShowLog(_T("ͬ���û�ֻ��ѡ������,��ѡ����������!"));
					return FALSE;

					CMyDataBase::GetInstance()->Close();
				}
				else if (m_mapTreeCtrToID[m_treeCtrl_curItem] != _T(""))//1.ֻѡ���� ����
				{

					CStringArray strAryDeviceID; //ѡ�������������
					FindChildID(strAryDeviceID, m_treeCtrl_curItem);
					//2.ϵͳѡ��
					//��ʼ���м����
					m_updateMysqlData.RemoveAll();

					CStringArray* pStrAry = nullptr;
					int			  strArySize = 0;
					GetArrayItems(strAryDeviceID, pStrAry, strArySize);

					for (int index = 0; index < strArySize; ++index)
					{

						if(pStrAry[index].GetSize()>0)
						{
							//7.ͬ���� sys_user ��Ϣ
							if (!Synchrodata_sys_user(pStrAry[index], strPath))
							{
								ShowLog(_T("�豸���ͱ�(sys_user) -ͬ��ʧ��!"));
								ret = FALSE;
							}
						}

					}

					ReleasArrayItems(pStrAry, strArySize);
					//00.ͬ����ɺ�,����mysql ͬ��״̬
					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("���·�������ݿ� ͬ��״̬ʧ�� -ͬ��ʧ��!"));
						ret = FALSE;
					}

				}
			}
			if (radio_devices == m_radioChoose) //ͬ���豸
			{


				//20180620 ����һ�� ͬ������ ����
				Synchrodata_areas(strPath, m_tongbu_dir);

				if (m_mapTreeCtrToIDType[m_treeCtrl_curItem] != _T("")) //2.ϵͳѡ��
				{

					CStringArray strAryDeviceID; //ѡ���豸�����豸
					FindChildTypeID(strAryDeviceID, m_treeCtrl_curItem);
					//��ʼ���м����
					m_updateMysqlData.RemoveAll();

					CStringArray* pStrAry = nullptr;
					int			  strArySize = 0;
					GetArrayItems(strAryDeviceID, pStrAry, strArySize);


					for (int index = 0; index < strArySize; ++index)
					{
						//2.ϵͳѡ��
						if(pStrAry[index].GetSize()>0)
						{
							//2.ͬ���� device_info
							if (!Synchrodata_device_info(pStrAry[index], strPath, m_tongbu_dir,tongbu_fw_devices))
							{
								ShowLog(_T("�豸���ͱ�(device_info) -ͬ��ʧ��!"));
								ret = FALSE;
							}
						}
					}
					ReleasArrayItems(pStrAry, strArySize);
					//00.ͬ����ɺ�,����mysql ͬ��״̬

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("���·�������ݿ� ͬ��״̬ʧ�� -ͬ��ʧ��!"));
						ret = FALSE;
					}
				}
				else if (m_mapTreeCtrToID[m_treeCtrl_curItem] != _T(""))//1.ֻѡ���� ����
				{

					CStringArray strAryDeviceID; //ѡ�������������
					FindChildID(strAryDeviceID, m_treeCtrl_curItem);
					//��ʼ���м����
					m_updateMysqlData.RemoveAll();

					CStringArray* pStrAry = nullptr;
					int			  strArySize = 0;
					GetArrayItems(strAryDeviceID, pStrAry, strArySize);

					for (int index = 0; index < strArySize; ++index)
					{
						//2.ϵͳѡ��
						if (pStrAry[index].GetSize() > 0)
						{
							//2.ͬ���� device_info
							if (!Synchrodata_device_info(pStrAry[index], strPath,m_tongbu_dir,tongbu_fw_areas))
							{
								ShowLog(_T("�豸���ͱ�(device_info) -ͬ��ʧ��!"));
								ret = FALSE;
							}
						}
					}
					ReleasArrayItems(pStrAry, strArySize);

					//00.ͬ����ɺ�,����mysql ͬ��״̬

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("���·�������ݿ� ͬ��״̬ʧ�� -ͬ��ʧ��!"));
						ret = FALSE;
					}
				}
			}
			if (radio_work == m_radioChoose) //ͬ������
			{
				if (m_mapTreeCtrToIDType[m_treeCtrl_curItem] != _T("")) //2.ϵͳѡ��
				{
					CStringArray strAryDeviceID; //ѡ�������������
					FindChildTypeID(strAryDeviceID, m_treeCtrl_curItem);
					//��ʼ���м����
					m_updateMysqlData.RemoveAll();

					
					CStringArray* pStrAry = nullptr;
					int		strArySize = 0;
					GetArrayItems(strAryDeviceID, pStrAry, strArySize);

					for (int index = 0; index < strArySize; ++index)
					{
						//2.ϵͳѡ��
						if (pStrAry[index].GetSize() > 0)
						{
							
							//3.ͬ���� work_template
							//ģ��� ����work_type�� ֻͬ�� work_type ��Ҫͬ�������� ������ģ��
							if (!Synchrodata_work_template(pStrAry[index], strPath, m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_template) -ͬ��ʧ��!"));
								ret = FALSE;
							}
						
							//4.ͬ���� work_type
							if (!Synchrodata_work_type(pStrAry[index], strPath,m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_type) -ͬ��ʧ��!"));
								ret = FALSE;
							}
						
							//5.ͬ���� work_task
							if (!Synchrodata_work_task(pStrAry[index], strPath,m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_task) -ͬ��ʧ��!"));
								ret = FALSE;

							}
							
							//6.ͬ���� work_record
							if (!Synchrodata_work_record(pStrAry[index], strPath, m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_record) -ͬ��ʧ��!"));
								ret = FALSE;
							}
							
						}
				
					}
					//00.ͬ����ɺ�,����mysql ͬ��״̬
					ReleasArrayItems(pStrAry, strArySize);
					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("���·�������ݿ� ͬ��״̬ʧ�� -ͬ��ʧ��!"));
						ret = FALSE;
					}
				}
				else if (m_mapTreeCtrToID[m_treeCtrl_curItem] != _T(""))//1.ֻѡ���� ����
				{
				
					//��ʼ���м����
					m_updateMysqlData.RemoveAll();
					//0.��ʼ����������豸����
					CStringArray strAryDeviceID;
					FindChildTypeID(strAryDeviceID, m_treeCtrl_curItem);

					CTime tm = CTime::GetCurrentTime();
					
					
					CStringArray* pStrAry = nullptr;
					int			  strArySize = 0;
					GetArrayItems(strAryDeviceID, pStrAry, strArySize);

					for (int index = 0; index < strArySize; ++index)
					{
						//2.ϵͳѡ��
						if (pStrAry[index].GetSize() > 0)
						{
					
							//3.ͬ���� work_template
							//ģ��� ����work_type�� ֻͬ�� work_type ��Ҫͬ�������� ������ģ��
							if (!Synchrodata_work_template(pStrAry[index], strPath, m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_template) -ͬ��ʧ��!"));
								ret = FALSE;
							}
					
							//4.ͬ���� work_type
							if (!Synchrodata_work_type(pStrAry[index], strPath, m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_type) -ͬ��ʧ��!"));
								ret = FALSE;
							}
					
							//5.ͬ���� work_task
							if (!Synchrodata_work_task(pStrAry[index], strPath, m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_task) -ͬ��ʧ��!"));
								ret = FALSE;

							}
							
							//6.ͬ���� work_record
							if (!Synchrodata_work_record(pStrAry[index], strPath, m_tongbu_dir))
							{
								ShowLog(_T("�豸���ͱ�(work_record) -ͬ��ʧ��!"));
								ret = FALSE;
							}
						
						}
						
					}
					//00.ͬ����ɺ�,����mysql ͬ��״̬
					ReleasArrayItems(pStrAry, strArySize);
					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("���·�������ݿ� ͬ��״̬ʧ�� -ͬ��ʧ��!"));
						ret = FALSE;
					}
			
				}
				


			}


			
			CMyDataBase::GetInstance()->Close();

				return ret;
			}
		else
		{
			ShowLog(_T("����ʧ��,�����²����豸������!"));
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
//����һ��Ϊɾ������,���Խ��лع�����
BOOL   CWPD_MTP_dataDlg::BeginRebackData(CString const& strFilePath)
{
	ShowLog(_T("��ʼ�ָ����ݿ�!"));
	CString strRebackPath = strFilePath;
	if (!PathFileExists(strRebackPath))
	{
		ShowLog(CString("�ָ�ʧ��:��Ч�ļ�·��-") + strRebackPath);
		return FALSE;
	}
	if (!CopyFile(strRebackPath, _T(".//") + m_fileName,FALSE))
	{
		CString strErr;
		strErr.Format(_T("�ָ�ʧ��:�����м��ļ�ʧ��err=%d"), GetLastError());
		ShowLog(strErr);
		return FALSE;
	}
	strRebackPath = _T(".//") + m_fileName;
	BOOL ret;
	IPortableDevice* pDevice = nullptr; //����ʹ�õ��豸
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
	
	if (strDevID != m_clearDeviceID)
	{
		ShowLog(_T("�ָ�ʧ��:����ͻָ����豸��һ��"));
		return FALSE;
	}
	HRESULT hr = S_OK;

	gDirID = _T(""), gFileID = _T("");
	//gDevice = nullptr;

	////////////////////////////////////
	WCHAR buffErr[1024] = { 0 };
	if (!OpenDevice(strDevID, &pDevice) && pDevice == nullptr)
	{
		ShowLog(_T("�豸����ʧ��,��ȡ�豸ʧ��!"));
		return FALSE;
	}


	IPortableDeviceContent*  content = nullptr;
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

	for (int index = 0; index < m_aryFileName.GetSize(); ++index)
	{
		strPath += m_aryFileName.GetAt(index) + _T("#");
		if (index == 0) //��һ��
		{
			PCWSTR objID = getSpecifiedObjectID(pDevice, m_aryFileName.GetAt(index), buffErr);
			dirID = objID;
			CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2597dirID[") + dirID + _T("]#err") + buffErr + _T("#end"));
			continue;;
		}

		if (index == 1) //�ڶ���
		{
			PCWSTR objID = getIDByParentID(pDevice, content, dirID, m_aryFileName.GetAt(index) + _T(".db"));
			fileID = objID;
			CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2604fileID[") + fileID + _T("]#end"));
			continue;
		}
		//����2��֮��
		PCWSTR objID = getIDByParentID(pDevice, content, fileID, m_aryFileName.GetAt(index));
		CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2609fileID[") + objID + _T("]#end"));
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


	//��ʼ�ع�
	//1.ɾ���ն��ϵ����ݿ�,�������ݵ����ݿ⵽�ն�
	
	DeleteDataFromDevice(pDevice, gFileID);
	
	ret = TransferDataToDevice(pDevice, WPD_CONTENT_TYPE_ALL, gDirID, strRebackPath, buffErr);

	CLogRecord::WriteRecordToFile(buffErr);
	if (pDevice)
	{
		pDevice->Close();
		pDevice->Release();
		pDevice = nullptr;
	}
	if (ret)
	{
		ShowLog(_T("�ָ����ݿ�ɹ�!"));
	}
	return ret;
}

//�ɹ���pc��mysql���ݿ� �������� д���м�sqlite���ݿ�, ���Ƹ����ݿ⵽ phone��
BOOL  CWPD_MTP_dataDlg::BeginPcToPhone(IPortableDevice* & pDevice)
{
	BOOL ret = FALSE;

	WCHAR buffErr[1024] = { 0 };
	//ɾ���ֳ��豸�ϵ�phone.db
	DeleteDataFromDevice(pDevice, gFileID);
	//MoveContentAlreadyOnDevice(pDevice, gFileID, gDirID);
// 	UpdateContentOnDevice(pDevice,
// 		WPD_CONTENT_TYPE_FOLDER,
// 		L"*.*\0*.*\0\0",
// 		nullptr, gDirID);
	//����phone.db���ֳ��豸��
	//gFilePath = _T(".//phone.db");
	
	//gFilePath = ;
	ret = TransferDataToDevice(pDevice, WPD_CONTENT_TYPE_ALL, gDirID, gFilePath, buffErr);
	
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
	//ɾ����ǰ���м��ļ�
	DeleteFile(gFilePath);
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
			PCWSTR objID = getSpecifiedObjectID(pDevice, m_aryFileName.GetAt(index), buffErr);
			dirID = objID;
			CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2597dirID[") + dirID + _T("]#err")+ buffErr + _T("#end"));
			continue;;
		}
	
		if (index == 1) //�ڶ���
		{
			PCWSTR objID = getIDByParentID(pDevice, content, dirID, m_aryFileName.GetAt(index)+_T(".db"));
			fileID = objID;
			CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2604fileID[") + fileID + _T("]#end"));
			continue;
		}
		//����2��֮��
		PCWSTR objID = getIDByParentID(pDevice, content, fileID, m_aryFileName.GetAt(index));
		CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2609fileID[") + objID + _T("]#end"));
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
	CString strName = Time.Format(_T("%Y-%m-%d-%H-%M-%S"));
	BOOL bRet = CopyFile(_T(".//") + m_fileName, _T(".//backUp//") + strName, FALSE);
	if (m_clearFlag)
	{
		m_copyName = strName;
		m_clearDeviceID = strDevID;
		m_clearFlag = FALSE;//���ñ�ʶ
	}
	else
	{
		m_clearDeviceID = _T("");
		m_copyName = _T("");
	}
	gFilePath = _T(".//") + m_fileName; //�м��ļ�·��
	
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

	//20180608 ���� ������ʾ
	CString strText; //��ʾ
	CString strTitle; //����


	if (pDlg->m_radioChoose == radio_work) //����
	{
		strText = _T("���ڽ���[ͬ������],����ȷ�Ͽ�ʼ!");
	}
	if (pDlg->m_radioChoose == radio_user) //�û�
	{
		strText = _T("���ڽ���[ͬ���û�],����ȷ�Ͽ�ʼ!");
	}
	if (pDlg->m_radioChoose == radio_devices) //�豸
	{
		strText = _T("���ڽ���[ͬ���豸],����ȷ�Ͽ�ʼ!");
	}
	if (pDlg->m_tongbu_dir == tongbu_to_phone)
	{
		strTitle = _T("��ʾ:��PCͬ�����ն�");
	}
	if (pDlg->m_tongbu_dir == tongbu_to_pc)
	{
		strTitle = _T("��ʾ:���ն�ͬ����PC");
	}
	if (IDYES == pDlg->MessageBox(strText,strTitle, MB_YESNO|MB_TOPMOST))
	{
	}
	else
	{
		pDlg->ShowLog(_T("ȡ��ͬ������"));
		pDlg->m_threadFlag = FALSE;
		pDlg->m_bt_tophone.EnableWindow(TRUE);
		pDlg->m_button_zdtopc.EnableWindow(TRUE);
		return -1;
	}

	//20180627 ���� ˢ�½���
	pDlg->m_button_flash.EnableWindow(FALSE);

	/************************************************************************/
#ifdef DEBUG
	CTime tm;
	tm = CTime::GetCurrentTime();
	pDlg->ShowLog(tm.Format(_T("%H%M%S")));
#endif
#ifdef MYTEST
	gFilePath = _T(".//safetydata.db"); //�м��ļ�·��
	//pDlg->DeleteAreaAndDevice(gFilePath);
	pDlg->BeginSwitchData(gFilePath);
	tm = CTime::GetCurrentTime();
	pDlg->ShowLog(_T("end")+tm.Format(_T("%H%M%S")));
	pDlg->m_threadFlag = FALSE;
	pDlg->m_bt_tophone.EnableWindow(TRUE);
	pDlg->m_button_zdtopc.EnableWindow(TRUE);
	pDlg->m_button_flash.EnableWindow(TRUE);
	if (pDlg->m_radioChoose == radio_user)//�û�ѡ��
	{
		pDlg->m_button_zdtopc.EnableWindow(FALSE);
	}
	return 0;
#endif
	//1.����phone���ݵ� pc����
	BOOL ret = FALSE;
	pDlg->ShowLog(_T("����ͬ��..."));
	 if (pDlg->BeginPhoneToPc(gDevice))
	 {
		 
		 //2.ͬ�����µ�sqlite���� ��pc mysql���ݿ�  ����ȡ���µ�mysql���� д��sqlite ���ݿ�

		 if (pDlg->BeginSwitchData(gFilePath))
		 {

			
			// 4.����phone�˵����ݿ�  ��pc�� �м�sqlite���ݿ� ������ phone��ָ��λ��
			 ret =  pDlg->BeginPcToPhone(gDevice);
		 }

	 }
	 if (gDevice)
	 {
		 gDevice->Close();
		 gDevice->Release();
		 gDevice = nullptr;
	 }

	

	pDlg->m_threadFlag = FALSE;
	pDlg->m_bt_tophone.EnableWindow(TRUE);
	pDlg->m_button_zdtopc.EnableWindow(TRUE);
	pDlg->m_button_flash.EnableWindow(TRUE);
	if (pDlg->m_radioChoose == radio_user)//�û�ѡ��
	{
		pDlg->m_button_zdtopc.EnableWindow(FALSE);
	}
	
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
//ͬ��pc���ն�
void CWPD_MTP_dataDlg::OnBnClickedBtTophone()
{	
	//�ж� ���ؼ�ѡ�� ΪҶ�ڵ�
#ifndef MYTEST

	if (nullptr == m_treeCtrl_curItem)
	{
		ShowLog(_T("��ѡ��һ����������豸"));
		return;
	}
#endif
	
	//MyControllingFunction(this);
	m_tongbu_dir = tongbu_to_phone;
	m_button_zdtopc.EnableWindow(FALSE);
	if (!m_threadFlag)
	{
		// TODO: �ڴ���ӿؼ�֪ͨ����������
		AfxBeginThread(MyControllingFunction, this);
		
		m_threadFlag = TRUE;
	}
	else
		ShowLog(_T("ͬ����æ,���ԵȻ������²���豸����!!"));
}
LRESULT CWPD_MTP_dataDlg::OnMyDeviceChange(WPARAM wParam, LPARAM lParam)
{
	if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) {
		PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
	
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

//�ݹ�ɾ���ļ����������ļ�
void DeleteDirectory(CString strDir)
{
	//����ɾ���ļ������ļ��� 
	CTime Time = CTime::GetTickCount();
	CTimeSpan saveTime(0, BACKUPSAVETIME,0,0);//������� ʱ��ı�������
	CString strName = (Time-saveTime).Format(_T("%Y-%m-%d-%H-%M-%S"));
	
	CFileFind   ff;

	BOOL bFound = ff.FindFile(strDir + _T("\\*"),0
	);
	CString fileName;
	while (bFound)
	{
		
		bFound = ff.FindNextFile();
		fileName = ff.GetFileName();
		if (fileName == _T(".") || fileName == _T("..")
			|| fileName == _T("˵��.txt") || fileName >= strName)
			continue;

		//ȥ���ļ�(��)ֻ�������� 

		SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL);

		if (ff.IsDirectory())
		{

			//�ݹ�ɾ�����ļ��� 

			DeleteDirectory(ff.GetFilePath());

			RemoveDirectory(ff.GetFilePath());

		}
		else
		{

			DeleteFile(ff.GetFilePath());   //ɾ���ļ� 

		}

	}

	ff.Close();

}

void CWPD_MTP_dataDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	//�������м��ļ�,��ɾ��
	if (!gFilePath.IsEmpty())
	{
#ifndef MYTEST
		//ɾ����ǰ���м��ļ�
		DeleteFile(gFilePath);
#endif
	}
	//�رմ򿪵��豸


	m_threadFlag = FALSE;
	ASSERT(m_threadShowDevs);
	ResumeThread(m_threadShowDevs->m_hThread);
	//CMyDataBase::GetInstance()->Close();

	//20180608  ���ǵ��������� ,��ʡ�洢�ռ�,��Ч�ļ�ɾ��
	//��������ļ�
	DeleteDirectory(_T(".//backUp"));


	//������ṹ
	ClearDeleteTreeStruct(m_deleteTree);


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

	//20180607 ��ʱ����
	return;

	m_treeCtrl_curItemType = nullptr;
	m_static_curchoose.SetWindowText(_T("��ǰ����:"));
	m_static_current_area.SetWindowText(m_mapArea[m_mapTreeCtrToID[m_treeCtrl_curItem]]);


	InitDeviceTree(m_mapTreeCtrToID[m_treeCtrl_curItem]);

	HTREEITEM hCurItem = m_tree_type.GetRootItem();

	while (hCurItem)
	{
		MyExpandTree(m_tree_type, hCurItem);
		hCurItem = m_tree_type.GetNextSiblingItem(hCurItem);
	}

}

//20180513
//����,ͬʱˢ�·���� ���µ��豸
void CWPD_MTP_dataDlg::OnBnClickedRefreshDevs()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	AfxBeginThread(Init_areas_tree_thread, this);


	ASSERT(m_threadShowDevs);
	ResumeThread(m_threadShowDevs->m_hThread);
	ShowLog(_T("�������,����ȡʧ��,�����²��һ���豸������!"));
}


void CWPD_MTP_dataDlg::OnTvnSelchangedTreeType(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	m_treeCtrl_curItemType = pNMTreeView->itemNew.hItem;
	m_static_current_area.SetWindowText(m_mapType[m_mapTreeCtrToIDType[m_treeCtrl_curItemType]]);
	m_static_curchoose.SetWindowText(_T("��ǰϵͳ:"));
}

//չ�����нڵ�
void CWPD_MTP_dataDlg::MyExpandTree(CTreeCtrl &treeCtrl, HTREEITEM hTreeItem)
{
	if (!treeCtrl.ItemHasChildren(hTreeItem))
	{
		return;
	}
	HTREEITEM hNextItem = treeCtrl.GetChildItem(hTreeItem);
	while (hNextItem != NULL)
	{
		MyExpandTree(treeCtrl,hNextItem);
		hNextItem = treeCtrl.GetNextItem(hNextItem, TVGN_NEXT);
	}
	treeCtrl.Expand(hTreeItem, TVE_EXPAND);
}

void CWPD_MTP_dataDlg::OnNMCustomdrawTreeAreas(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	LPNMTVCUSTOMDRAW pDraw = (LPNMTVCUSTOMDRAW)pNMHDR;
	
	DWORD dwDrawStage = pDraw->nmcd.dwDrawStage;
	UINT uItemState = pDraw->nmcd.uItemState;
	*pResult = CDRF_NOTIFYITEMDRAW;
	//|CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYSUBITEMDRAW|CDRF_NOTIFYPOSTERASE;
	CDC* pdc = CDC::FromHandle(pDraw->nmcd.hdc);
	CRect rc;
	HTREEITEM hItem = (HTREEITEM)pDraw->nmcd.dwItemSpec;
	if (hItem==nullptr)
	{
		return;
	}
	int nImage, nSelectedImage;
	bool icon = m_tree_areas.GetItemImage(hItem,nImage,nSelectedImage);
	m_tree_areas.GetItemRect(hItem, &rc, TRUE);//FALSE);text only
	CString txt = m_tree_areas.GetItemText(hItem);
	if ((dwDrawStage & CDDS_ITEM) && (uItemState & CDIS_SELECTED))
	{
// 		CDC mdc;
// 		mdc.CreateCompatibleDC(pdc);
// 		CBitmap bitm;
// 		HGDIOBJ bits = mdc.SelectObject((HBITMAP)m_treeIconList[nSelectedImage]);
// 		
// 		pdc->BitBlt(0, 0, 100, 100, &mdc, 0, 0, SRCCOPY);
		//return;
#ifdef TREESHOWICON
		COLORREF crText, crBkgnd;
			{
				crText = RGB(0, 0, 0);
				crBkgnd = RGB(49, 106, 197);

				pDraw->clrText = crText;  //����������ɫ  
				pDraw->clrTextBk = crBkgnd;  //���ñ�����ɫ  
			}
		return;
#endif

		pdc->FillSolidRect(&rc, RGB(49, 106, 197));//clr);
		pdc->SetTextColor(RGB(255, 255, 255));//white
		pdc->SetBkColor(RGB(49, 106, 197));//clr);
		CFont* pfnt = pdc->GetCurrentFont();
		POINT point;
		point.x = rc.left-32;
		point.y = rc.top;
		
		pdc->DrawIcon(point, m_treeIconList[nSelectedImage]);
		pdc->TextOut(rc.left + 2, rc.top + 2, txt);
		
		//pdc->DrawTextW(txt,rc, DT_LEFT&DT_BOTTOM);
		pdc->SelectObject(pfnt);
		*pResult |= CDRF_SKIPDEFAULT;
		// afxDump << "1\n";
	}
	else // without these ,1st blue !
	{
		pdc->FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));
		pdc->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		pdc->TextOut(rc.left + 2, rc.top + 2, txt);
		// afxDump << "2\n";
	}
}
void CWPD_MTP_dataDlg::OnNMCustomdrawTreeType(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	LPNMTVCUSTOMDRAW pDraw = (LPNMTVCUSTOMDRAW)pNMHDR;
	DWORD dwDrawStage = pDraw->nmcd.dwDrawStage;
	UINT uItemState = pDraw->nmcd.uItemState;
	*pResult = CDRF_NOTIFYITEMDRAW;
	//|CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYSUBITEMDRAW|CDRF_NOTIFYPOSTERASE;
	CDC* pdc = CDC::FromHandle(pDraw->nmcd.hdc);
	CRect rc;
	HTREEITEM hItem = (HTREEITEM)pDraw->nmcd.dwItemSpec;
	if (hItem == nullptr)
	{
		return;
	}
	m_tree_type.GetItemRect(hItem, &rc, TRUE);//FALSE);text only
	CString txt = m_tree_type.GetItemText(hItem);
	if ((dwDrawStage & CDDS_ITEM) && (uItemState & CDIS_SELECTED))
	{
		pdc->FillSolidRect(&rc, RGB(49, 106, 197));//clr);
		pdc->SetTextColor(RGB(255, 255, 255));//white
		pdc->SetBkColor(RGB(49, 106, 197));//clr);
		CFont* pfnt = pdc->GetCurrentFont();
		pdc->TextOut(rc.left + 2, rc.top + 2, txt);
		pdc->SelectObject(pfnt);
		*pResult |= CDRF_SKIPDEFAULT;
		// afxDump << "1\n";
	}
	else // without these ,1st blue !
	{
		pdc->FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));
		pdc->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		pdc->TextOut(rc.left + 2, rc.top + 2, txt);
		// afxDump << "2\n";
	}
}

//�ն��豸ͬ���� pc��
void CWPD_MTP_dataDlg::OnBnClickedButtonZdtopc()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�ж� ���ؼ�ѡ�� ΪҶ�ڵ�
	if (nullptr == m_treeCtrl_curItem)
	{
		ShowLog(_T("��ѡ��һ����������豸"));
		return;
	}
	//����

	//MyControllingFunction(this);
	m_tongbu_dir = tongbu_to_pc;
	m_bt_tophone.EnableWindow(FALSE);
	if (!m_threadFlag)
	{
		
		// TODO: �ڴ���ӿؼ�֪ͨ����������
		AfxBeginThread(MyControllingFunction, this);

		m_threadFlag = TRUE;
	}
	else
		ShowLog(_T("����ʧ��,�����²����豸������!!"));
}


void CWPD_MTP_dataDlg::OnBnClickedRadioUsrs()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_radioChoose = radio_user;
	m_button_zdtopc.EnableWindow(FALSE);
}


void CWPD_MTP_dataDlg::OnBnClickedRadioDevices()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_radioChoose = radio_devices;
	m_button_zdtopc.EnableWindow(TRUE);
}


void CWPD_MTP_dataDlg::OnBnClickedRadioWork()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_radioChoose = radio_work;
	m_button_zdtopc.EnableWindow(TRUE);
}



//��ʼ������߳�
UINT BeginClearThread(LPVOID pParam)
{
	CWPD_MTP_dataDlg* pDlg = (CWPD_MTP_dataDlg*)pParam;
	IPortableDevice* gDevice = nullptr; //����ʹ�õ��豸
	
	CString strBeginT, strEndT;
	CTime timeBegin,timeEnd;
	pDlg->m_datetime_begin.GetTime(timeBegin);
	pDlg->m_datetime_end.GetTime(timeEnd);

	if (timeBegin>timeEnd)
	{
		pDlg->ShowLog(_T("��ʼʱ�����С�ڻ��ߵ��ڽ���ʱ��!"));
		pDlg->ShowLog(_T("��ֹ�������!"));
		return -1;
	}


	strBeginT = timeBegin.Format(_T("%Y-%m-%d 00:00:00"));
	strEndT = timeEnd.Format(_T("%Y-%m-%d 23:59:59"));

#ifdef MYTEST
	gFilePath = _T(".//safetydata.db"); //�м��ļ�·��
	pDlg->Synchrodata_clear_record(gFilePath, strBeginT, strEndT);
	pDlg->ShowLog(_T("����������!"));
	return 0;
#endif


	BOOL ret = FALSE;
	if (pDlg->BeginPhoneToPc(gDevice))
	{

		//2.ɾ���ն����ݿ��� ѡ��ʱ��ļ�¼������
			if (pDlg->Synchrodata_clear_record(gFilePath,strBeginT,strEndT))
			{
				// 3.����phone�˵����ݿ�  ��pc�� �м�sqlite���ݿ� ������ phone��ָ��λ��
				ret = pDlg->BeginPcToPhone(gDevice);
			}
			
	}
	if (gDevice)
	{
		gDevice->Close();
		gDevice->Release();
		gDevice = nullptr;
	}
	if (ret)
	{
		pDlg->ShowLog(_T("����������!"));
	}
	else
		pDlg->ShowLog(_T("�������ʧ��!"));
	return ret;
}

void CWPD_MTP_dataDlg::OnBnClickedButtonBeginClear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_clearFlag = TRUE;
	if (IDYES == MessageBox(_T("����[��]��ʼ�������!"), _T("��ʾ:�������!"), MB_YESNO|MB_TOPMOST))
	{
		AfxBeginThread(BeginClearThread, this);
		ShowLog(_T("��ʼ�����������!"));
		return;
	}
	ShowLog(_T("ȡ���������!"));
}

UINT BeginRebackThread(LPVOID pParam)
{
	CWPD_MTP_dataDlg* pDlg = (CWPD_MTP_dataDlg*)pParam;
	IPortableDevice* gDevice = nullptr; //����ʹ�õ��豸

	BOOL ret = FALSE;
	ret = pDlg->BeginRebackData(_T(".//backUp//")+ pDlg->m_copyName);
	return ret;
}
void CWPD_MTP_dataDlg::OnBnClickedButtonReback()
{
	if (IDNO == MessageBox(_T("����[��]��ʼ�ָ�����!")
		, _T("��ʾ:�ָ�����!"), MB_YESNO | MB_TOPMOST))
	{
		ShowLog(_T("ȡ���ָ�����"));
		return;
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_copyName.IsEmpty())
	{
		ShowLog(_T("��һ�η��������,�޻ع�����!"));
		return;
	}
	AfxBeginThread(BeginRebackThread, this);
}

//����ʱ��
void CWPD_MTP_dataDlg::OnDtnDatetimechangeDatetimepickerEnd(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	static bool bMoCalOk = true;

	//�ж��Ƿ���CMonthCalCtrl�����лᷢ������Change,�صڶ��μ���
	CMonthCalCtrl* pMoCalCtrl = m_datetime_end.GetMonthCalCtrl();
	if (pMoCalCtrl != NULL)
	{
		bMoCalOk = !bMoCalOk;
	}


	if (!bMoCalOk)
	{
		return;
	}


	CTime beginTime, endTime;
	m_datetime_begin.GetTime(beginTime);
	m_datetime_end.GetTime(endTime);
	CTime curTime = CTime::GetCurrentTime();
	if (endTime < beginTime || curTime < endTime)
	{
		ShowLog(_T("����ʱ�������ڵ��ڿ�ʼʱ��,����С�ڵ��ڵ�ǰʱ��!"));
		m_bt_begin_clear.EnableWindow(FALSE);
		m_bt_begin_reback.EnableWindow(FALSE);
	}
	else
	{
		m_bt_begin_clear.EnableWindow(TRUE);
		m_bt_begin_reback.EnableWindow(TRUE);
	}
}

//��ʼʱ��
void CWPD_MTP_dataDlg::OnDtnDatetimechangeDatetimepickerBegin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	static bool bMoCalOk = true;

	//�ж��Ƿ���CMonthCalCtrl�����лᷢ������Change,�صڶ��μ���
	CMonthCalCtrl* pMoCalCtrl = m_datetime_begin.GetMonthCalCtrl();
	if (pMoCalCtrl != NULL)
	{
		bMoCalOk = !bMoCalOk;
	}


	if (!bMoCalOk)
	{
		return;
	}


	CTime beginTime, endTime;
	m_datetime_begin.GetTime(beginTime);
	m_datetime_end.GetTime(endTime);
	CTime curTime = CTime::GetCurrentTime();
	CString curdata = curTime.Format("%Y-%m-%d");
	if (endTime < beginTime || beginTime > curTime || endTime > curTime)
	{
		ShowLog(CString(_T("��ʼʱ�����С�ڵ��ڽ���ʱ��,\r\n����С�ڵ��ڵ�ǰʱ��!")));
		m_bt_begin_clear.EnableWindow(FALSE);
		m_bt_begin_reback.EnableWindow(FALSE);
	}
	else
	{
		m_bt_begin_clear.EnableWindow(TRUE);
		m_bt_begin_reback.EnableWindow(TRUE);
	}
}


