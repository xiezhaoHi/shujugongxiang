
// WPD_MTP_dataDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxdtctl.h"

//删除树结构
typedef struct deleteTree
{
	CString m_strID; //区域或者设备的ID
	CString m_parentID;//区域或者设备父亲ID
	CString m_strName; //设备或者区域名字
	int  m_typeFlag; //区域或者设备的标志0区域 1设备
	BOOL  m_deleteFlag; //删除标志  TRUE 删除 FALSE不删除
	struct deleteTree *m_fistchild;//第一个儿子
	struct deleteTree *m_nextsibling;//下一个兄弟
	deleteTree()
	{
		m_parentID = _T("0");
		m_strID = _T("0");
		m_strName = _T("");
		m_typeFlag = -1; //根节点
		m_deleteFlag = FALSE;
		m_fistchild = nullptr;
		m_nextsibling = nullptr;
	}
}deleteTree;
// CWPD_MTP_dataDlg 对话框
class CWPD_MTP_dataDlg : public CDialogEx
{
// 构造
public:
	CWPD_MTP_dataDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WPD_MTP_DATA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
	//树型控件 图片资源
	HICON		m_treeIconList[tree_max];
	CImageList	m_treeImageList;
	
	CTreeCtrl m_tree_areas; //区域选择树
	CStatic m_static_current_area; //当前选择区域
	CListBox m_listShow;
	BOOL	m_threadFlag; //线程标志 0 线程没启动,  1线程启动
	BOOL	m_showDevicesFlag; //刷新 存在的设备表
	CStringArray m_strDevicesID; //保存设备的ID
	CWinThread* m_threadShowDevs; //刷新设备列表的线程
	CComboBox m_combox_devices;  //选择设备
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapArea; //区域选择 ID-名字
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaParent; //区域 对应的 ID-父亲ID
	CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToID; //树控件对应的节点ID
	CMap< CString, LPCTSTR, HTREEITEM, HTREEITEM> m_mapTreeIDToCtrl; //树控件中 区域ID对应的 控件item;
	//////
	//work_type 关联 areas 表 展现在树形控件上
	CTreeCtrl m_tree_type;
	CMap< CString, LPCTSTR, HTREEITEM, HTREEITEM> m_mapAreaToCtr; //系统关联的树控件节点
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeToArea; //系统选择
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapType; //系统选择 ID-名字
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParent; //设备ID对应的父ID 
	CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToIDType; //树控件对应的节点ID
	
	CComboBox m_combox_chooseArea;
	HTREEITEM	m_treeCtrl_curItem; //区域树控件最新的选择项
	HTREEITEM	m_treeCtrl_curItemType; //系统work_type树控件最新的选择项
	//变量

	//20180607
	//单选框选中项
	int  m_radioChoose;

	//同步方向
	int m_tongbu_dir; //从pc到终端tongbu_to_phone = 1, 从终端到pc tongbu_to_pc
		
					  //20180608 中间备份数据库 方便删除操作 回滚
	CString m_copyName;

	//20180612 增加一个删除的区域和设备树结构
	deleteTree*	  m_deleteTree;
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaParentDelete; //区域 对应的 ID-父亲ID
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaStatusDelete; //区域 对应的 ID-同步状态  3 表示删除
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaNameDelete; //区域 对应的 ID-名字
	CMap<CString, LPCTSTR, deleteTree*, deleteTree*> m_mapAreaIDToNode; //区域ID对应的节点ID

	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeToAreaDelete; //系统选择ID-区域ID
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeParentDelete; //设备ID对应的父ID 
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeStatusDelete; //设备ID对应的-同步状态  3 表示删除
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapTypeNameDelete; //设备ID对应的-名字
private:
	CString m_strIni; //配置文件的路径
	MySQLConInfo  m_mysqlLogin; //mysql 数据库登陆 信息
	CStringArray m_aryFileName;//phone 上 文件的路径  分割成单个文件名
	CString m_fileName; //sqlite 文件名
	
	BOOL	m_clearFlag; //删除操作标识
	CString m_clearDeviceID; //删除操作的设备ID

	
	CList<CStringA> m_updateMysqlData; //更新mysql数据库 保存更新的sql语句.
	CList<CStringA>	m_areasDeviceID; // 缓存所选区域中间设备ID
	//方法
public:
	//配置文件初始化
	bool InitConfig();
	//初始化图片资源
	bool InitResource();
	//日志输出
	void ShowLog(CString const&);
	//等待设备 连接,连接后 获取指定路径下的文件,并备份到pc端
	BOOL  BeginPhoneToPc(IPortableDevice* &);

	//成功将pc端mysql数据库 最新数据 写入中间sqlite数据库, 复制该数据库到 phone端
	BOOL  BeginPcToPhone(IPortableDevice* &);

	//读取sqlite中的数据 传入mysql 数据库  读取mysql 数据库的数据 存入sqlite
	BOOL  BeginSwitchData(CString const& strPath);

	//20180608
	//若上一次为删除操作,可以进行回滚操作
	BOOL  BeginRebackData(CString const& );


	//work_type 建立相应的树
	BOOL    InitDeviceTree(CString const&);
	//获取相应区域的系统
	BOOL  InitWorkTypeMap(CString const& , CStringArray & );
	//展开树的所有节点
	void MyExpandTree(CTreeCtrl &treeCtrl,HTREEITEM hTreeItem);

	//////////////////////////////////////////////////////////////////////////
	//分表处理
	
	//关联的设备类型ID
	void FindChildTypeID(CStringArray & , HTREEITEM &);
	//选择的区域及所有子区域
	void FindChildID(CStringArray & strAry, HTREEITEM & item);
	//区域树 初始化
	void FindChildTree(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent);
	//work_type 关联的 区域 在树控件上展示
	void  FindChildTreeType(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent);
	//初始化该区域的设备缓存
	//20180607
	/*
	strAryDeviceID 返回设备
	*/
	BOOL Init_area_devices(CStringArray const& strAryAreaID, CStringArray & strAryDeviceID);
	//更新mysql数据库
	BOOL UpdateMysqlDB(CList<CStringA> const& list);

	//主要初始 区域选择combox
	BOOL Synchrodata_areas();
	
	//device_type 表处理
	BOOL  Synchrodata_device_type(CString const&  strAreaID, CString const& strDBPath);

	//device_info 表处理
	/*20180607 新增字段 
	*tongbuDir 同步方向:pc到终端 终端到pc
	*tongbuFw  同步范围: 区域   设备
	*/

	BOOL  Synchrodata_device_info(CString const&  strAreaID, CString const& strDBPath, int tongbuDir,int tongbuFw);

	//work_type  表处理
	/*20180607 新增字段
	*tongbuDir 同步方向:pc到终端 终端到pc
	*/
	BOOL  Synchrodata_work_type(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//work_template  表处理
	/*20180607 新增字段
	*tongbuDir 同步方向:pc到终端 终端到pc
	*/
	BOOL  Synchrodata_work_template(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//work_task  表处理
	/*20180607 新增字段
	*tongbuDir 同步方向:pc到终端 终端到pc
	*/
	BOOL Synchrodata_work_task(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//work_record  表处理
	/*20180607 新增字段
	*tongbuDir 同步方向:pc到终端 终端到pc
	*/
	BOOL  Synchrodata_work_record(CString const&  strAreaID, CString const& strDBPath, int tongbuDir);

	//sys_user 表处理
	BOOL  Synchrodata_sys_user(CString const&  strAreaID, CString const& strDBPath);

	//20180608
	//删除历史数据 根据选择的时间范围 删除相应的 记录和历史
	BOOL  Synchrodata_clear_record(CString const& strDBPath,CString const& strBeginT,CString const& strEndt);

	//////////////////////////////////////////////////////////////////////////
	//20180612 删除操作

	//初始化删除树的内存结构体
	BOOL  InitDeleteTreeStruct();

	//递归删除树结构
	BOOL  ClearDeleteTreeStruct(deleteTree* &);

	//递归创建区域树结构
	BOOL  CreateDeleteTreeStructAreas(deleteTree* &);
	//构建子树
	BOOL  CreateDeleteTreeStructAreasC(deleteTree* &);
	//递归创建设备树结构
	BOOL  CreateDeleteTreeStructDevices(deleteTree* &);

	//删除相应的区域或者设备
	BOOL  DeleteAreaAndDevice(CString const&);
	
	//递归获取 需要删除的区域和设备 包括他们的子区域和设备
	BOOL  DeleteAreaAndDeviceFindID(deleteTree* &, CStringArray&, CStringArray&);

	//递归获取需要删除项的子节点 及子节点的兄弟节点
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
