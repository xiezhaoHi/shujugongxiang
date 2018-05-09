
// WPD_MTP_dataDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"



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
	/*                                                                      */
	/************************************************************************/

	CTreeCtrl m_tree_areas; //区域选择树
	CStatic m_static_current_area; //当前选择区域
	CListBox m_listShow;
	BOOL	m_threadFlag; //线程标志 0 线程没启动,  1线程启动
	BOOL	m_showDevicesFlag; //刷新 存在的设备表
	CStringArray m_strDevicesID; //保存设备的ID
	CWinThread* m_threadShowDevs; //刷新设备列表的线程
	CComboBox m_combox_devices;  //选择设备
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapArea; //区域选择
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapAreaParent; //区域 对应的 父亲ID
	CMap<HTREEITEM, HTREEITEM, CString, LPCTSTR> m_mapTreeCtrToID; //树控件对应的节点ID
	CComboBox m_combox_chooseArea;
	HTREEITEM	m_treeCtrl_curItem; //树控件最新的选择项
	//变量
private:
	CString m_strIni; //配置文件的路径
	MySQLConInfo  m_mysqlLogin; //mysql 数据库登陆 信息
	CStringArray m_aryFileName;//phone 上 文件的路径  分割成单个文件名
	CString m_fileName; //sqlite 文件名
	
	CList<CStringA> m_updateMysqlData; //更新mysql数据库 保存更新的sql语句.
	CList<CStringA>	m_areasDeviceID; // 缓存所选区域中间设备ID
	//方法
public:
	//配置文件初始化
	bool InitConfig();
	//日志输出
	void ShowLog(CString const&);
	//等待设备 连接,连接后 获取指定路径下的文件,并备份到pc端
	BOOL  BeginPhoneToPc(IPortableDevice* &);

	//成功将pc端mysql数据库 最新数据 写入中间sqlite数据库, 复制该数据库到 phone端
	BOOL  BeginPcToPhone(IPortableDevice* &);

	//读取sqlite中的数据 传入mysql 数据库  读取mysql 数据库的数据 存入sqlite
	BOOL  BeginSwitchData(CString const& strPath);


	//////////////////////////////////////////////////////////////////////////
	//分表处理
	
	//区域树 初始化
	void FindChildTree(CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM>& mapParent);

	//初始化该区域的设备缓存
	BOOL Init_area_devices(CString const& strAreaID);

	//更新mysql数据库
	BOOL UpdateMysqlDB(CList<CStringA> const& list);

	//主要初始 区域选择combox
	BOOL Synchrodata_areas();
	
	//device_type 表处理
	BOOL  Synchrodata_device_type(CString const&  strAreaID, CString const& strDBPath);

	//device_info 表处理
	BOOL  Synchrodata_device_info(CString const&  strAreaID, CString const& strDBPath);

	//work_type  表处理
	BOOL  Synchrodata_work_type(CString const&  strAreaID, CString const& strDBPath);

	//work_template  表处理
	BOOL  Synchrodata_work_template(CString const&  strAreaID, CString const& strDBPath);

	//work_task  表处理
	BOOL Synchrodata_work_task(CString const&  strAreaID, CString const& strDBPath);

	//work_record  表处理
	BOOL  Synchrodata_work_record(CString const&  strAreaID, CString const& strDBPath);

	//sys_user 表处理
	BOOL  Synchrodata_sys_user(CString const&  strAreaID, CString const& strDBPath);
	//////////////////////////////////////////////////////////////////////////

	afx_msg void OnBnClickedRefreshDevs();
};
