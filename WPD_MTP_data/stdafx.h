
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

#ifdef DEBUG
//���Կ���
#define  MYTEST 
#endif
//���οؼ�  ��ʾͼ�����
#define  TREESHOWICON

//�������ʱ������� Сʱ
#define BACKUPSAVETIME 24

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
#include "verity/MyVerify.h"

//update ���ݿ�ʱ ����sql��� ���� ����ִ��
#define  UPDATEMAX 100 

//����ʱ���ڰ�����
#define VALIDDAY  180 

//���ױ�ʶ
#define  PARENTFLAG _T("0")

//areas ����� ÿ���ֶεı�ʶ
enum  areas_enum
{
	areas_Id		  ,
	areas_Code		  ,
	areas_Name		  ,
	areas_ParentId	  ,
	areas_CreateDate  ,
	areas_CreateUserId,
	areas_SynchronState
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
	 device_info_ParentId,
	 device_info_IsBindRFID
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
	work_task_AllId, 
	work_task_ExecutorTime,
	work_task_ExecutorLevel,
	work_task_TaskRemark,
	work_task_OverdueRemark,
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
	sys_user_Id,
	sys_user_LoginName,
	sys_user_Name,
	sys_user_Password,
	sys_user_Role,
	sys_user_AreaName,
	sys_user_SynchronState,
	sys_user_pwd_key,
	sys_user_role_id,


};

//���οؼ����� work_type
enum work_type_tree
{
	work_type_tree_Id,
	work_type_tree_ParentId,
	work_type_tree_Name,
	work_type_tree_AreaId,
	work_type_tree_SynchronState,
	work_type_tree_max
};

//��Դ
enum tree_icon
{
	tree_areas,
	tree_device,
	tree_max
};
//��ѡ��ѡ����
enum radio_name
{
	radio_user,
	radio_devices,
	radio_work
};

//ͬ������
enum tongbu_dir
{
	tongbu_to_phone = 1,
	tongbu_to_pc
};

//ͬ����Χѡ��
enum tongbu_fw
{
	tongbu_fw_areas,
	tongbu_fw_devices
};

//ɾ������ID
enum clear_work
{
	clear_work_id
};

//ɾ������ �豸 ����ص����� �ͼ�¼
//�豸��ʾ

#define  DELETEFLAG _T("3")

enum delete_type
{
	delete_type_areas,
	delete_type_devices
};

//20180614 �û���Ϣ
enum user_infoData
{
	user_infoData_name, //�û���
	user_infoData_pwd,
	user_infoData_area, //�û����������
	user_infoData_pwk, //������
	user_infoData_realName, //��ʾ����
};

//20180620 �����
enum areas_data
{
	areas_data_Id = 0,
	areas_data_Code,
	areas_data_Name,
	areas_data_ParentId,
	areas_data_CreateDate,
	areas_data_CreateUserId,
	areas_data_Info,
	areas_data_SynchronState,
	areas_data_isDelete,
};
//�ն����ݿ� areas �ֶ�����
#define PHONE_AREAS_NUM  6