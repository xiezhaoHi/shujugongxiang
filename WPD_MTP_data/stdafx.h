
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//���Կ���
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


//update ���ݿ�ʱ ����sql��� ���� ����ִ��
#define  UPDATEMAX 100 

//����ʱ���ڰ�����
#define VALIDDAY  180 

//areas ����� ÿ���ֶεı�ʶ
enum  areas_enum
{
	areas_Id		  ,
	areas_Code		  ,
	areas_Name		  ,
	areas_ParentId	  ,
	areas_CreateDate  ,
	areas_CreateUserId,
};

//device_type ��
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
//device_info ��
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

//work_type  ��
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

//work_template ��
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

//work_task ��
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

//work_record ��
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