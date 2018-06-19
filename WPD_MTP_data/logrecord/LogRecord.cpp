#include "..//stdafx.h"
#include "LogRecord.h"
#include <string>
#include <vector>

#include "shlwapi.h"  
#pragma comment(lib,"shlwapi.lib") 
/*exe����*/
#define  _EXELOG
CLogRecord CLogRecord::singleton;

CLogRecord::CLogRecord(void)
{

}


CLogRecord::~CLogRecord(void)
{
	if (m_logFile.m_hFile != CFile::hFileNull)
	{
		m_logFile.Close();
	}
}

#ifdef _EXELOG
CString  CLogRecord::ReturnCALLPath()  
{   
	CString    sPath;   
	GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
	sPath.ReleaseBuffer();   
	int    nPos;   
	nPos=sPath.ReverseFind(_T('\\'));   
	sPath=sPath.Left(nPos);   
	return    sPath;   
}
#else
CString  CLogRecord::ReturnOCXPath()  
{   
	CString    sPath;   
	GetModuleFileName(GetModuleHandle(_T("MakeInvActiveXCtr.ocx")),sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
	sPath.ReleaseBuffer();   
	int    nPos;   
	nPos=sPath.ReverseFind('\\');   
	sPath=sPath.Left(nPos);   
	return    sPath;   
}
#endif
BOOL CLogRecord::InitLogRecord()
{
	singleton.m_apppath = ReturnCALLPath();


	/*��ʱ ����*/
	{
		CString strIniPath = singleton.m_apppath +_T("\\logfiles\\day.ini");
		if (!PathFileExists(strIniPath))
		{
			::WritePrivateProfileString(_T("clear"),_T("day"),_T("30"),strIniPath);
		}
		singleton.ClearLog(::GetPrivateProfileInt(_T("clear"),_T("day"), 30,strIniPath));
	}
	CString logStr;
	logStr.Format(_T("%s\\logfiles"), singleton.m_apppath);

	if (!PathFileExists(logStr))
	{
		if (!CreateDirectory(_T("logfiles"), NULL))
		{
			return FALSE;
		}
	}
	singleton.m_logFileDir = logStr;
	return TRUE;
}

void CLogRecord::WriteRecordToFile(CString strLog)
{



	CString logStr;
	CString strDir = CTime::GetCurrentTime().Format("%Y-%m-%d");
	logStr.Format(_T("%s\\%s"), singleton.m_logFileDir, strDir);
	if (PathIsDirectory(logStr))
	{
		logStr.Format(_T("%s\\%s\\%s.log"), singleton.m_logFileDir,strDir, CTime::GetCurrentTime().Format("%Y-%m-%d-%H"));
	}
	else
	{
		BOOL bRet = ::CreateDirectory(logStr, NULL);//����Ŀ¼,���еĻ���Ӱ��  
		if (bRet)
		{
			logStr.Format(_T("%s\\%s\\%s.log"), singleton.m_logFileDir, strDir, CTime::GetCurrentTime().Format("%Y-%m-%d-%H"));
		}
		else
		{
			logStr.Format(_T("%s\\%s.log"), singleton.m_logFileDir, CTime::GetCurrentTime().Format("%Y-%m-%d-%H"));
		}
	}
	
	
	//���ڵ�ǰʱ��� ��־�ļ�
	if (PathFileExists(logStr)  )
	{
		//�ļ�δ��
		if (singleton.m_logFile.m_hFile == CFile::hFileNull)
		{
			//���ļ�,ʧ�ܾ��˳�
			if (!singleton.m_logFile.Open(logStr, CFile::modeWrite | CFile::shareDenyNone))
			{
				return;
			}
		}
	}
	else //�ļ�������
	{
		//�����Դ򿪵��ļ�
		if (singleton.m_logFile.m_hFile != CFile::hFileNull)
		{
			singleton.m_logFile.Close();
		}

		//���� �������ļ�
		if (!singleton.m_logFile.Open(logStr, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone))
		{
			return ;
		}
	}

	//д��־ǰ�ٴ� �ж��Ƿ� ���ļ��Դ�
	//�ļ�ָ�� �Ƶ��ļ�β
	if (singleton.m_logFile.m_hFile != CFile::hFileNull)
	{
		singleton.m_logFile.SeekToEnd();
	}
	else //�����ھͷ��� 
		return;


	std::string str;
	CString strCurDate;
	strCurDate = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S:\r\n");
	strCurDate += strLog;
	//	singleton.logFile.Write(strCurDate,strCurDate.GetLength());
#ifdef UNICODE
	int len = WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strCurDate, -1, NULL, 0, NULL, NULL);
	char *ptxtTemp = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strCurDate, -1, ptxtTemp, len, NULL, NULL);
	str = ptxtTemp;
	str += "\r\n";
	singleton.m_logFile.Write(str.c_str(), str.length());
	delete ptxtTemp;
#else
	singleton.m_logFile.Write(strCurDate, strCurDate.GetLength());
	singleton.m_logFile.Write("\r\n", 2);
#endif // UNICODE

}

CString CLogRecord::GetAppPath()
{
	return singleton.m_apppath;
}

/*
date: 20160921
author: xiezhao
add: �������� ��־���� 
����˵��: "0"��ʾֻ��������� "1"��ʾ �������� ... "5" ��ʾ����ǰ5��� �Դ�����
*/
BOOL CLogRecord::ClearLog(int day)
{
	CString logStr;
	CTime curTime = CTime::GetCurrentTime() - CTimeSpan( day, 0, 0, 0 );
	logStr.Format(_T("%s\\logfiles"), singleton.m_apppath);

	std::vector<CString> vec;
	TraverseDir(logStr,vec);
	for(std::vector<CString>::const_iterator it = vec.begin(); it < vec.end(); ++it)
	{
		CString strPath = *it;
		CString strLog = strPath.Right(strPath.GetLength() - strPath.ReverseFind(_T('\\'))-1);
		if (strLog < curTime.Format(_T("%Y-%m-%d.log")))
		{
			DeleteFile(strPath);
		}
	}
	return TRUE;
}

void CLogRecord::TraverseDir(CString& dir, std::vector<CString>& vec)
{
	CFileFind ff;
	if (dir.Right(1) != "\\")
		dir += "\\";
	dir += "*.*";

	BOOL ret = ff.FindFile(dir);
	while (ret)
	{
		ret = ff.FindNextFile();
		if (ret != 0)
		{
			if (ff.IsDirectory() && !ff.IsDots())
			{
				CString path = ff.GetFilePath();
				TraverseDir(path, vec);
			}
			else if (!ff.IsDirectory() && !ff.IsDots())
			{
				CString name = ff.GetFileName();
				CString path = ff.GetFilePath();
				vec.push_back(path);
			}
		}
	}
}
