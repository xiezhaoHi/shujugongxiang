
// WPD_MTP_dataDlg.cpp : 实现文件
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


// CWPD_MTP_dataDlg 对话框
// Device enumeration
DWORD EnumerateAllDevices();
void ChooseDevice(_Outptr_result_maybenull_ IPortableDevice** device);
BOOL ChooseDevice(
	_Outptr_result_maybenull_ IPortableDevice** device, CString strDevID, PWSTR buffErr);
//新增 获取所有的设备并返回  bufferr保存错误信息 wchar buffErr[1024] = {0};
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

//获取ShouChiZhongDuan文件夹ID
PCWSTR getSpecifiedObjectID(_In_ IPortableDevice* device, PCWSTR fileName,TCHAR* bufferr);
void TransforDataFromDevice(_In_ IPortableDevice* device, _In_ PCWSTR objectID);
map<wstring, wstring> childIDs;    //存放需要传递的文件的对象ID<->名称
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

	ChooseDevice(&device);  //默认第一个设备

	if (device == nullptr)
	{
		// No device was selected, so exit immediately.
		return;
	}

	//获取ShouChiZhongDuan文件夹的ID
	PCWSTR objID = _T("");// = getSpecifiedObjectID(device.Get(), L"音乐");  //koudaigouwu.apk
															   //获取ShouChiZhongDuan/data和ShouChiZhongDuan/data/phone.db的ID
	ComPtr<IPortableDeviceContent>  content;
	hr = device->Content(&content);
	if (FAILED(hr))
	{
		wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
		return;
	}
	PCWSTR dataID = getIDByParentID(device.Get(), content.Get(), objID, L"data");
	PCWSTR phoneID = getIDByParentID(device.Get(), content.Get(), dataID, L"phone");
	//删除手持设备上的phone.db
	DeleteDataFromDevice(device.Get(), phoneID);
	//拷贝phone.db到手持设备上
	//TransferDataToDevice(device.Get(), WPD_CONTENT_TYPE_ALL, dataID,gFilePath);



	CoTaskMemFree(eventCookie);
}


/************************************************************************/
//共享变量


CString	gDirID, gFileID; //phone端 sqlite 数据库存放位置ID  和 文件的ID
CString gFilePath; //中间文件的 路径
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
	DDX_Control(pDX, IDC_TREE_TYPE, m_tree_type);
	DDX_Control(pDX, IDC_STATIC_CurChoose, m_static_curchoose);
	DDX_Control(pDX, IDC_BUTTON_ZDTOPC, m_button_zdtopc);
	DDX_Control(pDX, IDC_BT_TOPHONE, m_bt_tophone);
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
END_MESSAGE_MAP()


// CWPD_MTP_dataDlg 消息处理程序
//不断刷新 所有连接的设备
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
		//pDlg->ShowLog(_T("扫描设备信息!"));
		ASSERT(pDlg->m_threadShowDevs);
		SuspendThread(pDlg->m_threadShowDevs->m_hThread);
	}
	return 0;
}


//树控件选择 显示
static const GUID GUID_DEVINTERFACE_LIST[] = { 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };

BOOL CWPD_MTP_dataDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	/************************************************************************/
	//对于wm_devicechange消息 注册 不注册 不会相应

	CLogRecord::WriteRecordToFile(_T("开始初始化工作!"));

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
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//1.初始化 配置
	if (!InitConfig())
	{
		ShowLog(_T("配置文件初始化失败!"));
	}
	else
		ShowLog(_T("配置文件初始化成功!"));

	//初始化资源
	if (!InitResource())
	{
		ShowLog(_T("资源初始化失败!"));
	}


	//初始化区域tree
	Synchrodata_areas();

	//默认选中任务
	((CButton*)GetDlgItem(IDC_RADIO_WORK))->SetCheck(1);
	m_radioChoose = radio_work;

	//优先级为“一般”的线程，默认栈大小，创建时挂起 CREATE_SUSPENDED
	m_threadShowDevs = AfxBeginThread(ShowAllDevices, this,THREAD_PRIORITY_NORMAL, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWPD_MTP_dataDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
//配置文件初始化
bool CWPD_MTP_dataDlg::InitConfig()
{
	CString strIni = CLogRecord::GetAppPath() + _T("//config//config.ini");
	
	string strAecKey = "0123456789ABCDEF0123456789ABCDEF";//256bits, also can be 128 bits or 192bits  
	string aesIV = "ABCDEF0123456789";//128 bits
	
	CStringA strPath = CpublicFun::UnicodeToAsc(strIni);
	m_strIni = strIni;
	char buff[MAX_PATH] = { 0 };

	CStringA aecFlag; //aec 加密标识  0未加密  1加密

	GetPrivateProfileStringA(("DATABASE"), ("aec"), "0"
		, buff, MAX_PATH, strPath);
	aecFlag = buff;
	std::memset(buff, 0, sizeof(buff));

	//MyAec myaec;
	string strOut; //中间变量 缓存 加密返回的数据
// 	if (aecFlag == "1") //已经加密过了
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
// 	else //第一次加密	////加密 数据库 连接信息 保存	
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
		//修改为加密 状态 1
		//WritePrivateProfileStringA(("DATABASE"), ("aec"), "1", strPath);
	}
	//////////////////////////////////////////////////////////////////////////
	//分割字符串
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

//初始化图片资源
bool CWPD_MTP_dataDlg::InitResource()
{
#ifndef TREESHOWICON
	return true;
#endif
	m_treeImageList.Create(32, 32, ILC_COLOR32, 3, 3);
// 	CString path = CLogRecord::GetAppPath() + _T("//resource//device.ico");
// 	HICON hIcon = (HICON)::LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
// 	int err = GetLastError();
	m_treeIconList[tree_system] = (HICON)::LoadImage(NULL, CLogRecord::GetAppPath() + _T("//resource//system.ico")
		, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	m_treeImageList.Add(m_treeIconList[tree_system]);

	m_treeIconList[tree_device] = (HICON)::LoadImage(NULL, CLogRecord::GetAppPath() + _T("//resource//device.ico")
		, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	m_treeImageList.Add(m_treeIconList[tree_device]);

	if (m_treeImageList.GetImageCount()>0)
	{
		m_tree_areas.SetImageList(&m_treeImageList, TVSIL_NORMAL);  // 建立 imagelist 与 tree的映射关系
		return true;
	}
	return false;
}

//日志输出
void CWPD_MTP_dataDlg::ShowLog(CString const& strLog)
{
	m_listShow.AddString(strLog);
	CLogRecord::WriteRecordToFile(strLog);
}

//寻找 子节点 并 加入到树控件
//参数为 父节点 和 对应树控件的 句柄
void  CWPD_MTP_dataDlg::FindChildTreeType(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent)
{
	//1.返回 当 没有父亲 就说明全是 叶节点 返回
	if (mapParent.GetSize() <= 0)
	{
		return;
	}
	//2.找 相应父亲的直接 子节点
	CString strKey,strValue;
	HTREEITEM valueItem;
	CString parentKey;
	
	POSITION posParent = mapParent.GetStartPosition();

		while (posParent)
		{
			mapParent.GetNextAssoc(posParent, parentKey, valueItem);
			CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItem; //父节点 对应的 树控件节点 句柄
			POSITION pos = m_mapTypeParent.GetStartPosition();
			while (pos)
			{
				m_mapTypeParent.GetNextAssoc(pos, strKey, strValue);//遍历 所有的 节点				
				if (parentKey == strValue) //当该节点为父节点
				{
					HTREEITEM item = m_tree_areas.InsertItem(m_mapType[strKey],1,1, valueItem);// 在根结点上添加Parent
					mapParentItem[strKey] = item; //构建新的 父节点
					m_mapTreeCtrToIDType[item] = strKey;
				}
			}
			//递归 获取
			FindChildTreeType(mapParentItem);
		}
}


//寻找 子节点 并 加入到树控件
//参数为 父节点 和 对应树控件的 句柄
void  CWPD_MTP_dataDlg::FindChildTree(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent)
{
	//1.返回 当 没有父亲 就说明全是 叶节点 返回
	if (mapParent.GetSize() <= 0)
	{
		return;
	}
	//2.找 相应父亲的直接 子节点
	CString strKey, strValue;
	HTREEITEM valueItem;
	CString parentKey;

	POSITION posParent = mapParent.GetStartPosition();

	while (posParent)
	{
		mapParent.GetNextAssoc(posParent, parentKey, valueItem);
		CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItem; //父节点 对应的 树控件节点 句柄
		POSITION pos = m_mapAreaParent.GetStartPosition();
		while (pos)
		{
			m_mapAreaParent.GetNextAssoc(pos, strKey, strValue);//遍历 所有的 节点				
			if (parentKey == strValue) //当该节点为父节点
			{
				HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey],0,0, valueItem);// 在根结点上添加Parent
				mapParentItem[strKey] = item; //构建新的 父节点
				m_mapTreeCtrToID[item] = strKey;
				m_mapTreeIDToCtrl[strKey] = item;
			}
		}
		//递归 获取
		FindChildTree(mapParentItem);
	}
}


//更新mysql数据库
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
			strTemp = ""; //重置中间变量
			sqlNum = 0;
			continue;
		}
		else
		{
			strTemp += list.GetNext(posUpdate);
		}
		//pDC->TextOut(200, 200, );//假设代码是在View类中的OnDraw()

	}
	BOOL ret = TRUE;
	if (strTemp != "")
	{
		ret = CMyDataBase::GetInstance()->Query(strTemp);
	}
	else if (strTemp != "" && sqlNum == 1)
	{
		ret= CMyDataBase::GetInstance()->OneQuery(strTemp);
	}
	if (!ret)
	{
		CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(strTemp.c_str()));
	}
	return ret;
}

//设备 建立相应的树 并关联区域
BOOL  CWPD_MTP_dataDlg::InitDeviceTree(CString const& strAreaID)
{
	//1.初始化缓存
	m_mapType.RemoveAll();
	m_mapTypeParent.RemoveAll();
	m_mapTypeToArea.RemoveAll();
	m_tree_type.DeleteAllItems();
	m_mapTreeCtrToIDType.RemoveAll(); //重置

	std::vector<std::vector<std::string> > vecDataType; //mysql 数据库的数据
														//2.获取 mysql数据库的 work_type表
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
				// 					CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapType; //类型选择
				// 					CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParent; //类型 对应的 父亲ID
				// 					CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToIDType; //树控件对应的节点ID
				//区域选择 保存ID
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
			ShowLog(_T("mysql数据库连接失败,请确保连接无误并重启软件!"));
			//关闭数据库
			CMyDataBase::GetInstance()->Close();
			return FALSE;
		}
	}
	else
	{
		CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
		CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
		ShowLog(_T("mysql数据库连接失败,请确保连接无误并重启软件!"));
		return FALSE;
	}
	//关闭数据库
	CMyDataBase::GetInstance()->Close();
	//work_type 关联区域
	POSITION posType = m_mapTypeParent.GetStartPosition();
	CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItemType; //父节点 对应的 树控件节点 句柄
	CString strKeyType, strValueType;
	while (posType)
	{
		m_mapTypeParent.GetNextAssoc(posType, strKeyType, strValueType);
		if (strValueType == PARENTFLAG) //为父亲节点
		{
			HTREEITEM itemType = m_tree_type.InsertItem(m_mapType[strKeyType], TVI_ROOT);// 在根结点上添加Parent
			mapParentItemType[strKeyType] = itemType;
			m_mapTreeCtrToIDType[itemType] = strKeyType;

		}
	}
	FindChildTreeType(mapParentItemType);
	return TRUE;
}


//获取区域相关的work_type  信息
BOOL  CWPD_MTP_dataDlg::InitWorkTypeMap(CString const& strAreaID,CStringArray & strAryID)
{

	std::vector<std::vector<std::string> > vecDataType; //mysql 数据库的数据
														//2.获取 mysql数据库的 work_type表
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
			ShowLog(_T("mysql数据库连接失败,请确保连接无误并重启软件!"));
			//关闭数据库
			//CMyDataBase::GetInstance()->Close();
			return FALSE;
		}
	}
// 	else
// 	{
// 		CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
// 		CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
// 		ShowLog(_T("mysql数据库连接失败,请确保连接无误并重启软件!"));
// 		return FALSE;
// 	}
	//关闭数据库
	//CMyDataBase::GetInstance()->Close();

	return TRUE;
}

//初始化 区域combox
BOOL CWPD_MTP_dataDlg::Synchrodata_areas()
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
		m_mapTreeCtrToIDType.RemoveAll(); //重置


		//////////////////////////////////////////////////////////////////////////

		std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
														//2.获取 mysql数据库的 区域表

		std::vector<std::vector<std::string> > vecDataType; //mysql 数据库的数据
															//2.获取 mysql数据库的 work_type表
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
					//区域选择 保存ID
					strName = CpublicFun::AscToUnicode(varVec[areas_Name].c_str());
					m_mapArea[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] = strName;
					//m_combox_chooseArea.AddString(strName);
					m_mapAreaParent[CpublicFun::AscToUnicode(varVec[areas_Id].c_str())] = CpublicFun::AscToUnicode(varVec[areas_ParentId].c_str());
				}

				//m_combox_chooseArea.SetCurSel(0);
			}
			else
			{
				CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
				ShowLog(_T("mysql数据库连接失败,请确保连接无误并重启软件!"));
				//关闭数据库
				CMyDataBase::GetInstance()->Close();
				return FALSE;
			}
			//////////////////////////////////////////////////////////////////////////
			//device_info
			CStringA strSql;
			strSql.Format("SELECT `Id`,\
					`ParentId`,\
					`Name`,\
					`AreaId`\
					FROM `device_info` where  SynchronState='0' or  SynchronState='1';");
			if (CMyDataBase::GetInstance()->Select(strSql.GetBuffer(), vecDataType))
			{
				int index = 0;
				CString strName;
				for each (std::vector<std::string> varVec in vecDataType)
				{
					// 					CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapType; //类型选择
					// 					CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParent; //类型 对应的 父亲ID
					// 					CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToIDType; //树控件对应的节点ID
					//区域选择 保存ID
					strName = CpublicFun::AscToUnicode(varVec[work_type_tree_Name].c_str());
					m_mapType[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())] = strName;
					//m_combox_chooseArea.AddString(strName);
					m_mapTypeParent[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
						= CpublicFun::AscToUnicode(varVec[work_type_tree_ParentId].c_str());
// 					m_mapTypeToArea[CpublicFun::AscToUnicode(varVec[work_type_tree_Id].c_str())]
// 						= CpublicFun::AscToUnicode(varVec[work_type_tree_AreaId].c_str());
				}

				//m_combox_chooseArea.SetCurSel(0);
			}

		}
		else
		{
			CStringA err = CMyDataBase::GetInstance()->GetErrorInfo();
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(err));
			ShowLog(_T("mysql数据库连接失败,请确保连接无误并重启软件!"));
			return FALSE;
		}

		//关闭数据库
		CMyDataBase::GetInstance()->Close();


		//////////////////////////////////////////////////////////////////////////
		//区域建树
		DWORD dwStyles = m_tree_areas.GetStyle();//获取树控制原风格  
		dwStyles |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS| TVS_SHOWSELALWAYS;//
		m_tree_areas.ModifyStyle(0, dwStyles);
		//初始化 树控件
		//1.先插入 父亲节点
		POSITION pos = m_mapAreaParent.GetStartPosition();
		CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItem; //父节点 区域ID对应的 树控件节点 句柄
		CString strKey, strValue;
		while (pos)
		{
			m_mapAreaParent.GetNextAssoc(pos, strKey, strValue);
			if (strValue == PARENTFLAG) //为父亲节点
			{
				HTREEITEM item = m_tree_areas.InsertItem(m_mapArea[strKey],0,0, TVI_ROOT);// 在根结点上添加Parent
				mapParentItem[strKey] = item;
				m_mapTreeCtrToID[item] = strKey;
				m_mapTreeIDToCtrl[strKey] = item;
				//m_mapAreaToCtr[strKey] = item; //保存区域 关联的树控件 节点
			}
		}
		//递归构建 区域 树
		FindChildTree(mapParentItem);

		
		//////////////////////////////////////////////////////////////////////////
		//设备建树
		//device_info 设备表关联区域
		//1.设备为根节点的情况
		POSITION posType = m_mapTypeParent.GetStartPosition();
		CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM> mapParentItemType; //父节点 对应的 树控件节点 句柄
		CString strKeyType, strValueType;
		while (posType)
		{
			m_mapTypeParent.GetNextAssoc(posType, strKeyType, strValueType);
			if (strValueType == PARENTFLAG) //为父亲节点
			{
				HTREEITEM itemType = m_tree_areas.InsertItem(m_mapType[strKeyType],1,1,TVI_ROOT);// 在根结点上添加Parent
				mapParentItemType[strKeyType] = itemType;
				m_mapTreeCtrToIDType[itemType] = strKeyType;

			}
		}
		FindChildTreeType(mapParentItemType);
		
		//2.设备为子节点的情况
		FindChildTreeType(m_mapTreeIDToCtrl);
		
		HTREEITEM hCurItem = m_tree_areas.GetRootItem();

		//展开所有节点
		while (hCurItem)
		{
			MyExpandTree(m_tree_areas, hCurItem);
			hCurItem = m_tree_areas.GetNextSiblingItem(hCurItem);
		}

		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}

/*同步work_record表数据,

1.同步phone端 记录数据到 pc端

需要根据 选择区域 的设备ID 来找
strAreaID  选择区域ID 暂时不用
新增需求:双向同步 
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_record(CString const&  strAreaID, CString const& strDBPath, int tongbuDir)
{
	try {
		

		//限定一个时间范围
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
		CString strSql;

		//从终端同步到pc
		if (tongbuDir == tongbu_to_phone)
		{
			//0.同步phone端的记录

			CList<CStringA> listUpdateSql; //保存需要更新的sql语句
			CSQLite sqOne;
			if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
			{
				int RFidCount = 0, RFidNum = 0;
				BOOL ret = FALSE;
				strSql.Format(_T("SELECT count(*) FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId='%s';")
					, strBeginTime, strCurTime, strAreaID);
				//char buf[MAX_PATH] = { 0 };
				if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
					//if (sqOne.QuickSelectDataCount(strSql,RFidCount))
				{
					//RFidCount = atoi(buf);
					if (RFidCount > 0)
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[RFidCount]; //同步到mysql数据库
						CStringArray** ppAryDataSqlite = new CStringArray*[RFidCount]; //更新phone端sqlite 数据 Id 和 同步状态
						strSql.Format(_T("SELECT `Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`Executor`,ClassName FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s' and  DeviceId='%s';")
							, strBeginTime, strCurTime, strAreaID);

						if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
						{
							//数据转换

							CString strRepTemp; //中间变量
							for (int index = 0; index < RFidCount; ++index)
							{
								ppAryDataSqlite[index] = new CStringArray;
								ppAryDataSqlite[index]->Add(ppAryData[index]->GetAt(work_record_Id));


								//做数据转换
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
									, "1"//同步状态 1已同步
									, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_CreateDate))
									, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_CreateUserId))
									, CpublicFun::UnicodeToAsc(ppAryData[index]->GetAt(work_record_ClassName))
								);
								listUpdateSql.AddTail(strTemp);
							}



							int retFlag = TRUE; //数据库操作
												//更新mysql数据库
							if (!UpdateMysqlDB(listUpdateSql))
							{
								ShowLog(_T("从phone更新记录数据服务端work_record失败!"));
								CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
								retFlag = false;
							}
							else
							{
								RFidNum = ppAryDataSqlite[0]->GetSize(); //获取表列数
																		 //更新phone端sqlite 状态
								if (!sqOne.QuickInsertData("UPDATE work_record set SynchronState ='1' where Id=?;", ppAryDataSqlite, RFidCount, RFidNum, CSQLite::sqlite3_bind))
								{
									CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr());
									retFlag = false;
								}
							}

							//销毁 缓存
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
							if (!retFlag)//上面的步骤错误
							{
								sqOne.CloseDataBase();
								return retFlag;
							}

						}
					}
				}
				sqOne.CloseDataBase();
				if (!ret)
				{
					CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + strSql);
					return FALSE;
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
				return FALSE;
			}


		}
		else if (tongbuDir == tongbu_to_phone) //从pc同步到终端
		{

			//同步服务端的数据

			strSql.Format(_T("SELECT `Id`,`DeviceId`,`WorkTaskId`,`Number`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,ClassName FROM `work_record` where SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
				, strBeginTime, strCurTime);
			//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;
						CList<CStringA> updateMysqlData;
						for each (std::vector<string> varVec in vecData)
						{
							//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
						//执行 没有就插入  有就更新
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
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
							return FALSE;
						}
						else //同步成功 更改状态
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
						CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
		}
		
		

		CLogRecord::WriteRecordToFile(_T("同步表[work_record]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}


/*同步work_task表数据,
1.pc端向phone端同步未同步的数据, 更改 同步状态.
2.表中同步状态为 已删除 则 删除phone端数据
3.同步phone端 完成状态和RFId

需要根据 选择区域 的设备ID 来找
strAreaID  选择区域ID 暂时不用
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_task(CString const&  strAreaID, CString const& strDBPath, int tongbuDir)
{
	try
	{

		//限定一个时间范围
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
		CString strSql;

		//设备ID
		CString strDeviceID=strAreaID;
		int sqlNum = 0;
	
		{
			//从终端同步到pc
			if (tongbuDir == tongbu_to_pc)
			{
				std::vector<std::vector<std::string> > vecDataTime; //mysql 数据库的数据
				strSql = _T("select now() as Systemtime;");
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecDataTime))
				{

				}
				if (vecDataTime.size() < 1)
				{
					CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
					return FALSE;
				}

				CStringArray** ppAryDataUpdate = nullptr;
				//0.同步phone端的 RFid 和 State字段
				int RFidCount = 0;
				CList<CStringA> listUpdateSql; //保存需要更新的sql语句
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
							strSql.Format(_T("SELECT `Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId` ,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark FROM `work_task` \
					where DeviceId='%s' and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';\
					"), strDeviceID, strBeginTime, strCurTime);
							if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
							{

								for (int index = 0; index < RFidCount; ++index)
								{

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
					sqOne.CloseDataBase();
					if (!ret)
					{
						CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + strSql);
						return FALSE;
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
					return FALSE;
				}

				//更新mysql数据库
				if (!UpdateMysqlDB(listUpdateSql))
				{
					ShowLog(_T("从phone更新RFId和完成状态到服务端work_task失败!"));
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));

					return FALSE;
				}
				else
				{
					if (ppAryDataUpdate)
					{
						if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
						{
							//更新phone 端数据 状态
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

			}
			else if (tongbuDir == tongbu_to_phone) //从pc同步到终端
			{
				std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
																//1.同步状态为 已删除，则删除phone端 
				strSql.Format(_T("SELECT `Id` FROM  `work_task` where SynchronState='2' and DeviceId ='%s' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
				vecData.clear();
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//有数据可以同步
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
								CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
								CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
								return FALSE;
							}
						}
						else
						{
							CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
							return FALSE;
						}
					}
				}
				else
				{

					CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					return FALSE;
				}



				strSql.Format(_T("SELECT `Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark FROM `work_task` \
		where DeviceId='%s' and SynchronState='0' and State<>'0' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
				//2.同步选择区域  未同步的数据  并把同步状态改为 已同步
				CList<CStringA> updateMysqlData;
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//有数据可以同步
					{
						CSQLite sq;
						if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[dataCount];
							int index = 0;

							for each (std::vector<string> varVec in vecData)
							{
								//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
							//执行 没有就插入  有就更新
							CStringA strSql = "Replace  INTO `work_task` (`Id`,`DeviceId`,`RFId`,`WorkName`,`WorkTemplateId`,`Cycle`,`StartTime`,`Executor`,`State`,`SynchronState`,`CreateDate`,`CreateUserId`,Level,Frequency,AllId,ExecutorTime,ExecutorLevel,TaskRemark,OverdueRemark) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

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
								CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
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
							CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
							return FALSE;
						}
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					return FALSE;
				}

			}

			

		}


		CLogRecord::WriteRecordToFile(_T("同步表[work_task]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}


/*同步work_template表数据,
1.pc端向phone端同步未同步的数据, 更改 同步状态.
2.表中同步状态为 已删除 则 删除phone端数据

需要根据 选择区域 的设备ID 来找
strAreaID  选择区域ID 暂时不用
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_template(CString const&  strAreaID, CString const& strDBPath, int tongbuDir)
{
	try
	{
		//work_template 只能从pc同步到终端
		if (tongbuDir == tongbu_to_pc)
		{
			return TRUE;
		}

		//限定一个时间范围
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
		CString strSql;
		CString strDeviceID = strAreaID;
		{
			std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
			//获取到的ID
			strSql.Format(_T("SELECT te.`Id`,te.`DeviceId`,te.`WorkTypeId`,te.ClassName,te.`Number`,te.`ProcessName`,te.`Results`,te.`SynchronState`,te.`CreateDate`,te.`CreateUserId` FROM `work_template` te,work_type tk \
		where te.SynchronState ='0' or te.SynchronState ='1' and   tk.DeviceId='%s'and tk.CreateDate >= '%s' and tk.CreateDate <= '%s' and tk.Id = te.WorkTypeId;\
		"), strDeviceID, strBeginTime, strCurTime);
			//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
			CList<CStringA> updateMysqlData;
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
						//执行 没有就插入  有就更新
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
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
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
						CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.同步状态为 已删除，则删除phone端 
			strSql.Format(_T("SELECT `Id` FROM  `work_template` where SynchronState='2' and DeviceId ='%s' and CreateDate >= '%s' and CreateDate <= '%s';\
		"), strDeviceID, strBeginTime, strCurTime);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
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
							CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

		}


		CLogRecord::WriteRecordToFile(_T("同步表[work_template]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}

}


/*同步work_type表数据,
1.pc端向phone端同步未同步的数据, 更改 同步状态.
2.表中同步状态为 已删除 则 删除phone端数据

strAreaID  选择区域ID 暂时不用
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_work_type(CString const&  strAreaID, CString const& strDBPath, int tongbuDir)
{
	try
	{
		//work_type 只能从pc同步到终端
		if (tongbuDir == tongbu_to_pc)
		{
			return TRUE;
		}

		//限定一个时间范围
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
		CString strSql;

		CString strDeviceID = strAreaID;
		{
			std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
			//获取到的ID
		
			strSql.Format(_T("SELECT `Id`,`Name`,`ProcessName`,`Results`,`SynchronState`,`CreateDate`,`CreateUserId`,DeviceId FROM `work_type`\
		 where SynchronState='0'  and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId='%s';\
		"), strBeginTime, strCurTime,strDeviceID);
			//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
			CList<CStringA> updateMysqlData;
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
						//执行 没有就插入  有就更新
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
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
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
						CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.同步状态为 已删除，则删除phone端 
			strSql.Format(_T("SELECT `Id` FROM  `work_type` where SynchronState='2' and CreateDate >= '%s' and CreateDate <= '%s' and DeviceId='%s';\
		"), strBeginTime, strCurTime,strDeviceID);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
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
							CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
			
		}
		CLogRecord::WriteRecordToFile(_T("同步表[work_type]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}


/*同步device_info表数据,
1.phone端 查询RFid 同步到mysql 数据库中
2.pc端向phone端同步未同步的数据, 更改 同步状态.
3.表中同步状态为 已删除 则 删除phone端数据
3.记录查询到的设备ID 方便查询 后续的表  中 对应设备的数据
strAreaID  选择区域ID
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_device_info(CString const&  strAreaID,CString const& strDBPath,int tongbuDir,int tongbuFw)
{
	try
	{
		//所选区域没有设备 

		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		CString strSql;
		//0.同步phone端的 RFid 字段
		//从终端到pc
		if (tongbu_to_pc == tongbuDir)
		{
			CList<CStringA> listUpdateSql; //保存需要更新的sql语句
			CSQLite sqOne;
			CStringArray** ppAryDataUpdate = nullptr;
			//0.同步phone端的 RFid 和 State字段
			int RFidCount = 0;
			if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
			{

				BOOL ret = FALSE;
				if (tongbu_fw_areas == tongbuFw) //同步范围:区域
				{
					strSql.Format(_T("SELECT count(*) FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
						, strAreaID, strBeginTime, strCurTime);
				}
				else if(tongbu_fw_devices == tongbuFw)//同步范围:设备
				{
					strSql.Format(_T("SELECT count(*) FROM `device_info` where Id='%s' and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
						, strAreaID, strBeginTime, strCurTime);
				}
				if (ret = sqOne.QuickSelectDataCount(CpublicFun::UnicodeToAsc(strSql), RFidCount))
				{
					if (RFidCount > 0)
					{
						CString strLog;
						strLog.Format(_T("同步的数据条数为[%d];"));
						CLogRecord::WriteRecordToFile(strSql + strLog);
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[RFidCount];
						ppAryDataUpdate = new CStringArray*[RFidCount];
						strSql.Format(_T("SELECT `Id`,`RFID` FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s' and SynchronState='0';")
							, strAreaID, strBeginTime, strCurTime);

						if (ret = sqOne.QuickSelectData(CpublicFun::UnicodeToAsc(strSql), ppAryData, RFidCount))
						{
							for (int index = 0; index < RFidCount; ++index)
							{
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
				sqOne.CloseDataBase();
				if (!ret)
				{
					CLogRecord::WriteRecordToFile(sqOne.GetLastErrorStr() + strSql);
					return FALSE;
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
				return FALSE;
			}

			//更新mysql数据库
			if (!UpdateMysqlDB(listUpdateSql))
			{
				ShowLog(_T("从phone更新RFId到服务端失败!"));
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
			else
			{
				if (ppAryDataUpdate)
				{
					if (sqOne.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						//更新phone 端数据 状态
						if (sqOne.QuickInsertData("update device_info set SynchronState='1' where Id = ?", ppAryDataUpdate, RFidCount, 1, CSQLite::sqlite3_bind))
						{

						}
						else
						{
							CLogRecord::WriteRecordToFile(_T("device_info 同步RFID 同步新数据操作失败!"));
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
		}
		else if (tongbu_to_phone == tongbuDir) //从pc到终端设备  同步设备表
		{
				//获取到的ID
				std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
				if (tongbu_fw_areas == tongbuFw) //同步范围:区域
				{
					strSql.Format(_T("SELECT `Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,\
		`SynchronState`,`CreateDate`,`CreateUserId` FROM `device_info` where AreaId='%s'and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
						, strAreaID, strBeginTime, strCurTime);
				}
				else if (tongbu_fw_devices == tongbuFw)//同步范围:设备
				{
					strSql.Format(_T("SELECT `Id`,`RFID`,`Name`,`Picture`,`TypeId`,`Info`,`AreaId`,\
		`SynchronState`,`CreateDate`,`CreateUserId` FROM `device_info` where Id='%s'and SynchronState='0' and CreateDate >= '%s' and CreateDate <= '%s';")
						, strAreaID, strBeginTime, strCurTime);
				}

				//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
				CList<CStringA> updateMysqlData;
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//有数据可以同步
					{
						CSQLite sq;
						if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
						{
							CStringA strTemp;
							CStringArray** ppAryData = new CStringArray*[dataCount];
							int index = 0;

							for each (std::vector<string> varVec in vecData)
							{
								//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
								++index;
							}
							dataNum = ppAryData[0]->GetSize();
							//执行 没有就插入  有就更新
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
								CLogRecord::WriteRecordToFile(sq.GetLastErrorStr() + CpublicFun::AscToUnicode(strSql));
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
							CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
							return FALSE;
						}
					}
				}
				else
				{
					CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					return FALSE;
				}

				//2.同步状态为 已删除，则删除phone端 
				if (tongbu_fw_areas == tongbuFw) //同步范围:区域
				{
					strSql.Format(_T("SELECT `Id` FROM `device_info` where AreaId='%s' and SynchronState='2' \
	and CreateDate >= '%s' and CreateDate <= '%s' ;"), strAreaID, strBeginTime, strCurTime);
				}
				else if (tongbu_fw_devices == tongbuFw)//同步范围:设备
				{
					strSql.Format(_T("SELECT `Id` FROM `device_info` where Id='%s' and SynchronState='2' \
	and CreateDate >= '%s' and CreateDate <= '%s' ;"), strAreaID, strBeginTime, strCurTime);
				}
				vecData.clear();
				if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
				{
					int dataCount = vecData.size(), dataNum = 0;
					if (dataCount > 0)//有数据可以同步
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
								CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
								CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
								return FALSE;
							}
						}
						else
						{
							CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
							return FALSE;
						}
					}
				}
				else
				{

					CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
					return FALSE;
				}
			}
		
		CLogRecord::WriteRecordToFile(_T("同步表[device_info]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}

/*同步device_type表数据,
1.pc端向phone端同步未同步的数据, 更改 同步状态.
2.表中同步状态为 已删除 则 删除phone端数据

strAreaID  选择区域ID
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_device_type(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{
		//20180607  取消系统的概念
		return TRUE;



		//限定一个时间范围
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
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
		//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
		CList<CStringA> updateMysqlData;
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//有数据可以同步
			{
				CSQLite sq;
				if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
				{
					CStringA strTemp;
					CStringArray** ppAryData = new CStringArray*[dataCount];
					int index = 0;

					for each (std::vector<string> varVec in vecData)
					{
						//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
					//执行 没有就插入  有就更新
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
					CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{
			CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}

		//2.同步状态为 已删除，则删除phone端 
		strSql.Format(_T("SELECT `Id` FROM  `device_type` where AreaId='%s' and SynchronState='2';"), strAreaID);
		vecData.clear();
		
		if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
		{
			int dataCount = vecData.size(), dataNum = 0;
			if (dataCount > 0)//有数据可以同步
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
						CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
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
					CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
					CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
					return FALSE;
				}
			}
		}
		else
		{

			CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
			CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
			return FALSE;
		}
		CLogRecord::WriteRecordToFile(_T("同步表[device_type]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}
//关联的设备类型ID
void CWPD_MTP_dataDlg::FindChildTypeID(CStringArray & strAry, HTREEITEM & item)
{
	//选择的设备类型 以及 所有的 子设备类型 关联的设备
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

//选择的区域及子区域ID
void CWPD_MTP_dataDlg::FindChildID(CStringArray & strAry, HTREEITEM & item)
{
	//选择的设备类型 以及 所有的 子设备类型 关联的设备
	///
	if (item == nullptr)
	{
		return;
	}
	if (m_mapTreeCtrToID[item] != _T("")) //有效区域
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

//初始化该区域的设备缓存
//strTypeId :strAreaID该区域下的   系统数组
BOOL CWPD_MTP_dataDlg::Init_area_devices(
	 CStringArray const& strAryAreaID, CStringArray & strAryDeviceID)
{
	try
	{
		strAryDeviceID.RemoveAll();
		//限定一个时间范围
		COleDateTime  curTime = COleDateTime::GetTickCount();
		COleDateTime  beginTime = curTime - COleDateTimeSpan(VALIDDAY);
		CString strCurTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CString strBeginTime = beginTime.Format(_T("%Y-%m-%d 00:00:00"));
		
// 		CStringArray strTypeId; //关联的系统ID
// 		//获取所选的系统 以及子系统 的所有ID
// 		FindChildTypeID(strTypeId,m_treeCtrl_curItemType);
		for(int indexAreaID = 0; indexAreaID < strAryAreaID.GetSize(); ++indexAreaID)
		{

			std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
			CString strSql;
			strSql.Format(_T("SELECT `Id` FROM `device_info` where AreaId='%s' and CreateDate >= '%s' and CreateDate <= '%s';")
				, strAryAreaID[indexAreaID], strBeginTime, strCurTime);
			//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
				{
					for each (std::vector<string> varVec in vecData)
					{
						//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库

						strAryDeviceID.Add(CpublicFun::AscToUnicode(varVec[0].c_str()));

					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}
		}

		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
	
}


/*同步sys_user表数据,
1.pc端向phone端同步未同步的数据, 更改 同步状态.
2.表中同步状态为 已删除 则 删除phone端数据

需要根据 选择区域 的设备ID 来找
strAreaID  选择区域ID 暂时不用
*/
BOOL  CWPD_MTP_dataDlg::Synchrodata_sys_user(CString const&  strAreaID, CString const& strDBPath)
{
	try
	{

		std::vector<std::vector<std::string> > vecData; //mysql 数据库的数据
		CString strSql;

		POSITION posDeviceID = m_areasDeviceID.GetHeadPosition();
		CString strDeviceID;
		int sqlNum = 0;

			strSql.Format(_T("SELECT u.id,u.user_name,u.real_name,u.password,r.role_name,a.Name, u.SynchronState ,u.pwd_key,u.role_id FROM `sys_user` u , sys_role r, areas a\
		where  u.SynchronState='0' or u.SynchronState='1' and u.data_areas = '%s' and u.data_areas = a.Id and u.role_id = r.id;\
		"), strAreaID);
			//1.同步选择区域  未同步的数据  并把同步状态改为 已同步
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
				{
					CSQLite sq;
					if (sq.OpenDataBase(CpublicFun::UnicodeToAsc(strDBPath)))
					{
						CStringA strTemp;
						CStringArray** ppAryData = new CStringArray*[dataCount];
						int index = 0;

						for each (std::vector<string> varVec in vecData)
						{
							//2.构建 更新 字符串 并保留起来,等到 同步完成 后 写入mysql数据库
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
						//执行 没有就插入  有就更新
						CStringA strSql = "Replace  INTO `user_table` (`Id`,`LoginName`,`Name`,`Password`,`Role`,`AreaName`,`SynchronState`,pwd_key,RoleId) VALUES(?,?,?,?,?,?,?,?,?);";

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
						CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{
				CLogRecord::WriteRecordToFile(_T("同步新数据操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

			//2.同步状态为 已删除，则删除phone端 
			strSql.Format(_T("SELECT `id` FROM  `sys_user` where SynchronState='2' and data_areas='%s';\
		"), strAreaID);
			vecData.clear();
			if (CMyDataBase::GetInstance()->Select(CpublicFun::UnicodeToAsc(strSql).GetBuffer(), vecData))
			{
				int dataCount = vecData.size(), dataNum = 0;
				if (dataCount > 0)//有数据可以同步
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
							ppAryData[index]->Add(CpublicFun::AscToUnicode(varVec[sys_user_Id].c_str()));

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
							CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
							CLogRecord::WriteRecordToFile(sq.GetLastErrorStr());
							return FALSE;
						}
					}
					else
					{
						CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
						CLogRecord::WriteRecordToFile(_T("打开中间数据库失败!") + strDBPath);
						return FALSE;
					}
				}
			}
			else
			{

				CLogRecord::WriteRecordToFile(_T("删除操作失败!") + strSql);
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}

		


		CLogRecord::WriteRecordToFile(_T("同步表[sys_user]操作完成!"));
		return TRUE;
	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}

}


/************************************************************************/
/*
	执行流程:	1.先从中间sqlite 数据库中 获取需要更新的数据,插入到pc端mysql数据库中,
				2.把pc端mysql数据库中的 需要更新的数据 拿出来 插入到中间sqlite数据库中

				同步数据 分区域处理 .只同步选定的区域
				功能实现 分表处理.
*/                                                                     
/************************************************************************/
BOOL  CWPD_MTP_dataDlg::BeginSwitchData(CString const& strPath)
{
	try
	{
		//初始化中间变量
		m_updateMysqlData.RemoveAll();

		m_areasDeviceID.RemoveAll();

		//连接mysql数据库
		if (CMyDataBase::GetInstance()->InitMyDataBase(m_mysqlLogin))
		{
			//选择的当前 区域ID 或者设备ID
			CString strAreaID;
			BOOL ret = TRUE;
			/*
			20180530
			修改:区域选择 或者系统选择
			1.区域选择 同步 该区域及所有子区域 下的所有系统
			2.系统选择 同步相应系统下的数据
			*/

			//20180607
			//同步根据所选的 同步任务来进行:同步用户  同步设备  同步任务
			//同步方向:从终端同步到pc端 和 从pc端同步到终端
			if (radio_user == m_radioChoose) //同步用户
			{
				//用户只有 从pc到 终端同步
				if (m_mapTreeCtrToIDType[m_treeCtrl_curItem] != _T("")) //2.系统选择
				{
					//用户只在区域下面
					ShowLog(_T("同步用户只能选择区域,请选择区域重试!"));
					return FALSE;

					CMyDataBase::GetInstance()->Close();
				}
				else if (m_mapTreeCtrToID[m_treeCtrl_curItem] != _T(""))//1.只选择了 区域
				{

					CStringArray strAryItem; //选择的区域及子区域
					FindChildID(strAryItem, m_treeCtrl_curItem);
					//2.系统选择
					//初始化中间变量
					m_updateMysqlData.RemoveAll();
					for (int index = 0; index < strAryItem.GetSize(); ++index)
					{

						strAreaID = strAryItem[index];
						CStringArray strTypeId;

						ASSERT(!strAreaID.IsEmpty());
						{

							//7.同步表 sys_user 信息
							if (!Synchrodata_sys_user(strAreaID, strPath))
							{
								ShowLog(_T("设备类型表(sys_user) -同步失败!"));
								ret = FALSE;
							}
						}

					}
					//00.同步完成后,更改mysql 同步状态

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("更新服务端数据库 同步状态失败 -同步失败!"));
						ret = FALSE;
					}

				}
			}
			if (radio_devices == m_radioChoose) //同步设备
			{

				if (m_mapTreeCtrToIDType[m_treeCtrl_curItem] != _T("")) //2.系统选择
				{

					CStringArray strAryItem; //选择设备及子设备
					FindChildTypeID(strAryItem, m_treeCtrl_curItem);
					//初始化中间变量
					m_updateMysqlData.RemoveAll();

					for (int index = 0; index < strAryItem.GetSize(); ++index)
					{
						//2.系统选择
						strAreaID = strAryItem[index];
						{
							//2.同步表 device_info
							if (!Synchrodata_device_info(strAreaID, strPath, m_tongbu_dir,tongbu_fw_devices))
							{
								ShowLog(_T("设备类型表(device_info) -同步失败!"));
								ret = FALSE;
							}
						}
					}
					//00.同步完成后,更改mysql 同步状态

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("更新服务端数据库 同步状态失败 -同步失败!"));
						ret = FALSE;
					}
				}
				else if (m_mapTreeCtrToID[m_treeCtrl_curItem] != _T(""))//1.只选择了 区域
				{

					CStringArray strAryItem; //选择的区域及子区域
					FindChildID(strAryItem, m_treeCtrl_curItem);
					//初始化中间变量
					m_updateMysqlData.RemoveAll();

					for (int index = 0; index < strAryItem.GetSize(); ++index)
					{
						//2.系统选择
						strAreaID = strAryItem[index];
						{
							//2.同步表 device_info
							if (!Synchrodata_device_info(strAreaID, strPath,m_tongbu_dir,tongbu_fw_areas))
							{
								ShowLog(_T("设备类型表(device_info) -同步失败!"));
								ret = FALSE;
							}
						}


					}


					//00.同步完成后,更改mysql 同步状态

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("更新服务端数据库 同步状态失败 -同步失败!"));
						ret = FALSE;
					}
				}
			}
			if (radio_work == m_radioChoose) //同步任务
			{
				if (m_mapTreeCtrToIDType[m_treeCtrl_curItem] != _T("")) //2.系统选择
				{
					CStringArray strAryItem; //选择的区域及子区域
					FindChildTypeID(strAryItem, m_treeCtrl_curItem);
					//初始化中间变量
					m_updateMysqlData.RemoveAll();

					for (int index = 0; index < strAryItem.GetSize(); ++index)
					{
						strAreaID = strAryItem[index];
						{

							//3.同步表 work_template
							//模板表 关联work_type表 只同步 work_type 需要同步的任务 关联的模板
							if (!Synchrodata_work_template(strAreaID, strPath, m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_template) -同步失败!"));
								ret = FALSE;
							}
							//4.同步表 work_type
							if (!Synchrodata_work_type(strAreaID, strPath,m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_type) -同步失败!"));
								ret = FALSE;
							}
							//5.同步表 work_task
							if (!Synchrodata_work_task(strAreaID, strPath,m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_task) -同步失败!"));
								ret = FALSE;

							}

							//6.同步表 work_record
							if (!Synchrodata_work_record(strAreaID, strPath, m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_record) -同步失败!"));
								ret = FALSE;
							}
						}
					}
					//00.同步完成后,更改mysql 同步状态

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("更新服务端数据库 同步状态失败 -同步失败!"));
						ret = FALSE;
					}
				}
				else if (m_mapTreeCtrToID[m_treeCtrl_curItem] != _T(""))//1.只选择了 区域
				{
					CStringArray strAryItem; //选择的区域及子区域
					FindChildID(strAryItem, m_treeCtrl_curItem);
					//初始化中间变量
					m_updateMysqlData.RemoveAll();
					//0.初始化该区域的设备缓存
					CStringArray strAryDeviceID;
					if (!Init_area_devices(strAryItem, strAryDeviceID))
					{
						ShowLog(_T("初始化该区域的设备缓存失败-同步失败!"));
						ret = FALSE;
					}
					for (int index = 0; index < strAryDeviceID.GetSize(); ++index)
					{
						
						strAreaID = strAryDeviceID[index];
						{

							//3.同步表 work_template
							//模板表 关联work_type表 只同步 work_type 需要同步的任务 关联的模板
							if (!Synchrodata_work_template(strAreaID, strPath, m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_template) -同步失败!"));
								ret = FALSE;
							}
							//4.同步表 work_type
							if (!Synchrodata_work_type(strAreaID, strPath, m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_type) -同步失败!"));
								ret = FALSE;
							}
							//5.同步表 work_task
							if (!Synchrodata_work_task(strAreaID, strPath, m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_task) -同步失败!"));
								ret = FALSE;

							}

							//6.同步表 work_record
							if (!Synchrodata_work_record(strAreaID, strPath, m_tongbu_dir))
							{
								ShowLog(_T("设备类型表(work_record) -同步失败!"));
								ret = FALSE;
							}
						}
					}
					//00.同步完成后,更改mysql 同步状态

					if (!UpdateMysqlDB(m_updateMysqlData))
					{
						ShowLog(_T("更新服务端数据库 同步状态失败 -同步失败!"));
						ret = FALSE;
					}
				}
				


			}


				CMyDataBase::GetInstance()->Close();
				return ret;
			}
			else
			{
				ShowLog(_T("连接失败,请重新插入设备再重试!"));
				CLogRecord::WriteRecordToFile(CpublicFun::AscToUnicode(CMyDataBase::GetInstance()->GetErrorInfo()));
				return FALSE;
			}


	}
	catch (...)
	{
		CString strLog;
		strLog.Format((_T("执行异常,最后错误代码为[%d]"), GetLastError()));
		CLogRecord::WriteRecordToFile(strLog);
		return FALSE;
	}
}


//成功将pc端mysql数据库 最新数据 写入中间sqlite数据库, 复制该数据库到 phone端
BOOL  CWPD_MTP_dataDlg::BeginPcToPhone(IPortableDevice* & pDevice)
{
	BOOL ret = FALSE;

	WCHAR buffErr[1024] = { 0 };
	//删除手持设备上的phone.db
	DeleteDataFromDevice(pDevice, gFileID);
	//MoveContentAlreadyOnDevice(pDevice, gFileID, gDirID);
// 	UpdateContentOnDevice(pDevice,
// 		WPD_CONTENT_TYPE_FOLDER,
// 		L"*.*\0*.*\0\0",
// 		nullptr, gDirID);
	//拷贝phone.db到手持设备上
	//gFilePath = _T(".//phone.db");
	
	//gFilePath = ;
	ret = TransferDataToDevice(pDevice, WPD_CONTENT_TYPE_ALL, gDirID, gFilePath, buffErr);
	
	CLogRecord::WriteRecordToFile(buffErr);
	//删除当前的中间文件
	DeleteFile(gFilePath);

	return ret;
}

//等待设备 连接,连接后 获取指定路径下的文件,并备份到pc端
BOOL CWPD_MTP_dataDlg::BeginPhoneToPc(IPortableDevice* & pDevice)
{
	
	int curIndex = m_combox_devices.GetCurSel();
	if (curIndex < 0)
	{
		ShowLog(_T("请选择一项设备"));
		return FALSE;
	}
	ASSERT(curIndex >= 0);
	ASSERT(curIndex < m_strDevicesID.GetSize());
	CString strDevID = m_strDevicesID.GetAt(curIndex); //当前选择的设备ID
													   //1.等待设备接入
													   //2.获取phone端的设备指定路径下的sqlite设备 
													   //(1)备份到本地 (2)获取库中 需要同步的数据
	HRESULT hr = S_OK;

	gDirID = _T(""), gFileID = _T("");
	//gDevice = nullptr;
	gFilePath = _T(".//") + m_fileName; //中间文件路径
	//删除当前的中间文件
	DeleteFile(gFilePath);
	////////////////////////////////////
	WCHAR buffErr[1024] = { 0 };
	if (!OpenDevice(strDevID, &pDevice) && pDevice == nullptr)
	{
		ShowLog(_T("设备连接失败,获取设备失败!"));
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

	CString dirID, fileID; //最后一个文件夹 ID 和 最终的数据库文件ID
	//获取sqlite数据库文件的ID
	CString strPath;
	
	for (int index =0; index < m_aryFileName.GetSize();++index)
	{
		strPath += m_aryFileName.GetAt(index) + _T("#");
		if (index ==0) //第一次
		{
			PCWSTR objID = getSpecifiedObjectID(pDevice, m_aryFileName.GetAt(index), buffErr);
			dirID = objID;
			CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2597dirID[") + dirID + _T("]#err")+ buffErr + _T("#end"));
			continue;;
		}
	
		if (index == 1) //第二次
		{
			PCWSTR objID = getIDByParentID(pDevice, content, dirID, m_aryFileName.GetAt(index)+_T(".db"));
			fileID = objID;
			CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2604fileID[") + fileID + _T("]#end"));
			continue;
		}
		//大于2次之后
		PCWSTR objID = getIDByParentID(pDevice, content, fileID, m_aryFileName.GetAt(index));
		CLogRecord::WriteRecordToFile(m_aryFileName.GetAt(index) + _T("#2609fileID[") + objID + _T("]#end"));
		dirID = fileID;
		fileID = objID;
	}
	if (dirID.IsEmpty() || fileID.IsEmpty())
	{
		
		ShowLog(_T("丢失数据库文件,请确保文件存在!----"));
		if (content)
		{
			content->Release();
		}
		return FALSE;
	}
	//保存当前使用文件ID
	gDirID = dirID;

	gFileID = fileID;
	
	TransforDataFromDevice(pDevice, fileID);
	//(1)先删除本地的中间文件, 复制phone 端文件, 备份sqlite 数据库 


	CTime Time = CTime::GetTickCount();
	CString strName = Time.Format(_T("%Y-%m-%d-%H-%M-%S"));
	BOOL bRet = CopyFile(_T(".//") + m_fileName, _T(".//backUp//") + strName, FALSE);
	gFilePath = _T(".//") + m_fileName; //中间文件路径
	ShowLog(_T("正在同步..."));
	if (content)
	{
		content->Release();
	}

	return TRUE;
}
/************************************************************************/
/*
注意服务:WPDBusEnum  (Portable Device Enumerator Service)

同步功能:
	说明:本地电脑有个mysql数据库,usb连接的android手机端 有一个sqlite 数据库.
同步功能分为1.phone端 同步sqlite中最新的数据 到pc端 mysql数据库 2.pc端 mysql 中最新的数据 同步到 
phone端 sqlite 数据库中.

	执行流程:1.线程等待 phone设备通过usb方式连接 pc端. 2.等到正确的设备连入,获取phone端的
	sqlite 数据库,拷贝到pc端.(1)做备份.以时间节点为名字.(2)将sqlite库中最新的数据写入pc端mysql数据库
	(3)获取pc端的mysql数据库中 需要更新的数据.并写入sqlite.(4)删除phone端的sqlite数据库,把新的sqlite数据库
	复制过去.

*/
/************************************************************************/
UINT MyControllingFunction(LPVOID pParam)
{
	//界面指针
	CWPD_MTP_dataDlg* pDlg = (CWPD_MTP_dataDlg*)pParam;
	IPortableDevice* gDevice = nullptr; //正在使用的设备

	/************************************************************************/
#ifdef DEBUG
	CTime tm;
	tm = CTime::GetCurrentTime();
	pDlg->ShowLog(tm.Format(_T("%H%M%S")));
#endif
#ifdef MYTEST
	gFilePath = _T(".//phone.db");
#endif
	//1.备份phone数据到 pc本地
	BOOL ret = FALSE;
	 if (pDlg->BeginPhoneToPc(gDevice))
	 {
		 
		 //2.同步最新的sqlite数据 到pc mysql数据库  并获取最新的mysql数据 写入sqlite 数据库

		 if (pDlg->BeginSwitchData(gFilePath))
		 {
			// 3.更新phone端的数据库  把pc端 中间sqlite数据库 拷贝到 phone端指定位置
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
	if (ret)
	{
		pDlg->ShowLog(_T("同步完成..."));
#ifdef DEBUG
		tm = CTime::GetCurrentTime();
		pDlg->ShowLog(tm.Format(_T("%H%M%S")));
#endif
	}
	else
		pDlg->ShowLog(_T("同步失败..."));
	return 0;
	/************************************************************************/
}
//同步pc到终端
void CWPD_MTP_dataDlg::OnBnClickedBtTophone()
{	
	//判断 树控件选择 为叶节点
	if (nullptr == m_treeCtrl_curItem)
	{
		ShowLog(_T("请选择一个区域"));
		return;
	}
	//测试
#ifdef MYTEST
	//gFilePath = _T(".//phone.db");
	BeginSwitchData(gFilePath);
	return;
#endif
	//MyControllingFunction(this);
	m_tongbu_dir = tongbu_to_phone;
	m_button_zdtopc.EnableWindow(FALSE);
	if (!m_threadFlag)
	{
		// TODO: 在此添加控件通知处理程序代码
		AfxBeginThread(MyControllingFunction, this);
		
		m_threadFlag = TRUE;
	}
	else
		ShowLog(_T("连接失败,请重新插入设备再重试!!"));
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
			//需要的操作  
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
	// TODO: 在此添加控件通知处理程序代码
	CString strRee = _T("");
	return;
}


void CWPD_MTP_dataDlg::OnInputDeviceChange(unsigned short nState, HANDLE hDevice)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnInputDeviceChange(nState, hDevice);
}


void CWPD_MTP_dataDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	
	CDialogEx::OnOK();
}


void CWPD_MTP_dataDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//若存在中间文件,则删除
	if (!gFilePath.IsEmpty())
	{
		//删除当前的中间文件
		DeleteFile(gFilePath);
	}
	//关闭打开的设备


	m_threadFlag = FALSE;
	ASSERT(m_threadShowDevs);
	ResumeThread(m_threadShowDevs->m_hThread);
	//CMyDataBase::GetInstance()->Close();
	CDialogEx::OnClose();
}


void CWPD_MTP_dataDlg::OnLbnDblclkListMsg()
{
	// TODO: 在此添加控件通知处理程序代码
	m_listShow.ResetContent();
}


void CWPD_MTP_dataDlg::OnTvnSelchangedTreeAreas(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);



	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	m_treeCtrl_curItem = pNMTreeView->itemNew.hItem;

	//20180607 暂时不用
	return;

	m_treeCtrl_curItemType = nullptr;
	m_static_curchoose.SetWindowText(_T("当前区域:"));
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
//新增,同时刷新服务端 更新的设备
void CWPD_MTP_dataDlg::OnBnClickedRefreshDevs()
{
	// TODO: 在此添加控件通知处理程序代码

	//初始化区域combox
	Synchrodata_areas();

	ASSERT(m_threadShowDevs);
	ResumeThread(m_threadShowDevs->m_hThread);
	ShowLog(_T("更新完成,若获取失败,请重新插拔一次设备再重试!"));
}


void CWPD_MTP_dataDlg::OnTvnSelchangedTreeType(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	m_treeCtrl_curItemType = pNMTreeView->itemNew.hItem;
	m_static_current_area.SetWindowText(m_mapType[m_mapTreeCtrToIDType[m_treeCtrl_curItemType]]);
	m_static_curchoose.SetWindowText(_T("当前系统:"));
}

//展开所有节点
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
	// TODO: 在此添加控件通知处理程序代码
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
	// TODO: 在此添加控件通知处理程序代码
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

//终端设备同步到 pc端
void CWPD_MTP_dataDlg::OnBnClickedButtonZdtopc()
{
	// TODO: 在此添加控件通知处理程序代码
	//判断 树控件选择 为叶节点
	if (nullptr == m_treeCtrl_curItem)
	{
		ShowLog(_T("请选择一个区域"));
		return;
	}
	//测试
#ifdef MYTEST
	//gFilePath = _T(".//phone.db");
	BeginSwitchData(gFilePath);
	return;
#endif
	//MyControllingFunction(this);
	m_tongbu_dir = tongbu_to_pc;
	m_bt_tophone.EnableWindow(FALSE);
	if (!m_threadFlag)
	{
		
		// TODO: 在此添加控件通知处理程序代码
		AfxBeginThread(MyControllingFunction, this);

		m_threadFlag = TRUE;
	}
	else
		ShowLog(_T("连接失败,请重新插入设备再重试!!"));
}


void CWPD_MTP_dataDlg::OnBnClickedRadioUsrs()
{
	// TODO: 在此添加控件通知处理程序代码
	m_radioChoose = radio_user;
	m_button_zdtopc.EnableWindow(FALSE);
}


void CWPD_MTP_dataDlg::OnBnClickedRadioDevices()
{
	// TODO: 在此添加控件通知处理程序代码
	m_radioChoose = radio_devices;
	m_button_zdtopc.EnableWindow(TRUE);
}


void CWPD_MTP_dataDlg::OnBnClickedRadioWork()
{
	// TODO: 在此添加控件通知处理程序代码
	m_radioChoose = radio_work;
	m_button_zdtopc.EnableWindow(TRUE);
}
