
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//测试开关
//#define  MYTEST 

#include "logrecord/LogRecord.h"
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <windows.h>
#include <commdlg.h>
#include <new>
#include <shlwapi.h>
#include <propvarutil.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;

#include <PortableDeviceApi.h>      // Include this header for Windows Portable Device API interfaces
#include <PortableDevice.h>         // Include this header for Windows Portable Device definitions
#include "CommonFunctions.h"        // Includes common prototypes for functions used across source files

#define SELECTION_BUFFER_SIZE 81    // Buffer size for user selection

#include "logrecord/publicFun.h"
#include "sqlite3/CSQLite.h"
#include "mysql/MyDataBase.h"


//update 数据库时 多条sql语句 连接 批量执行
#define  UPDATEMAX 100 

//创建时间在半年内
#define VALIDDAY  180 

//areas 区域表 每个字段的标识
enum  areas_enum
{
	areas_Id		  ,
	areas_Code		  ,
	areas_Name		  ,
	areas_ParentId	  ,
	areas_CreateDate  ,
	areas_CreateUserId,
};

//device_type 表
enum device_type_enum
{
	device_type_Id			  ,
	device_type_Name		  ,
	device_type_ParentId	  ,
	device_type_AreaId		  ,
	device_type_SynchronState ,
	device_type_CreateDate	  ,
	device_type_CreateUserId  ,

};
//device_info 表
enum device_info_enum
{
	device_info_Id			 ,
	device_info_RFID			 ,
	device_info_Name			 ,
	device_info_Picture		 ,
	device_info_TypeId		 ,
	device_info_Info			 ,
	device_info_AreaId		 ,
	device_info_SynchronState ,
	device_info_CreateDate	 ,
	device_info_CreateUserId	 ,
};

//work_type  表
enum work_type_enum
{
	work_type_Id		   ,
	work_type_Name		   ,
	work_type_ProcessName  ,
	work_type_Results	   ,
	work_type_SynchronState,
	work_type_CreateDate   ,
	work_type_CreateUserId ,

};

//work_template 表
enum work_template_enum
{
	work_template_Id			,
	work_template_DeviceId		,
	work_template_WorkTypeId	,
	work_template_ClassName,
	work_template_Number		,
	work_template_ProcessName	,
	work_template_Results		,
	work_template_SynchronState	,
	work_template_CreateDate	,
	work_template_CreateUserId	,
};

//work_task 表
enum MyEnum
{
	work_task_Id			,
	work_task_DeviceId		,
	work_task_RFId			,
	work_task_WorkName		,
	work_task_WorkTemplateId,
	work_task_Cycle			,
	work_task_StartTime		,
	work_task_Executor		,
	work_task_State			,
	work_task_SynchronState	,
	work_task_CreateDate	,
	work_task_CreateUserId	,
	work_task_Level,
	work_task_Frequency,
	work_task_max

};

//work_record 表
enum work_record_enum
{
	work_record_Id,
	work_record_DeviceId,
	work_record_WorkTaskId,
	work_record_Number,
	work_record_ProcessName,
	work_record_Results,
	work_record_SynchronState,
	work_record_CreateDate,
	work_record_CreateUserId,
	work_record_ClassName,
	work_record_max

};

//sys_user
enum sys_user_enum
{
	sys_user_Id			 ,
	sys_user_LoginName	 ,
	sys_user_Name		 ,
	sys_user_Password	 ,
	sys_user_Role		 ,
	sys_user_AreaName	 ,
	sys_user_SynchronState,
	sys_user_pwd_key

};