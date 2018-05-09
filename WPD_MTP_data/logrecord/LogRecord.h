#pragma once
#include <vector>
#include <atlstr.h>


class CLogRecord
{
public:
	~CLogRecord(void);
private:
	CLogRecord(void);
	CString		m_apppath;
	CString		m_logFileDir; //��־�ļ��� ·��
	static		CLogRecord	singleton;

public:
	static CString GetAppPath();
	static BOOL InitLogRecord();
	static void WriteRecordToFile(CString strLog);
	static CString ReturnOCXPath();
	static CString ReturnCALLPath();
private:
	CFile	m_logFile;

public:
	/*
	date: 20160921
	author: xiezhao
	add: �������� ��־���� 
	����˵��: "0"��ʾֻ��������� "1"��ʾ �������� ... "5" ��ʾ����ǰ5��� �Դ�����
	*/
	BOOL   ClearLog(int day);
	void   TraverseDir(CString& dir, std::vector<CString>& vec);
};


