#include "..\\stdafx.h"
#include "MyVerify.h"
#include "MyMd5.h"
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

MyVerify::MyVerify()
{
	m_strFileName = _T("KLD_license");
}


MyVerify::~MyVerify()
{
}
MyVerify* MyVerify::m_singleton = nullptr;
MyVerify* MyVerify::GetInstance()
{
	if (nullptr == m_singleton)
	{
		m_singleton = new MyVerify;
	}
	return m_singleton;
}
CStringA MyVerify::GetEncryptStr(CStringA const& strInput) //��ȡ���ܺ������
{
	CStringA strRet;
	if (strInput.IsEmpty())
	{
		//1.��ȡ���� mac
		if (GetLocoalIPandMac(m_strMac))
		{
			strRet = *(m_strMac.begin()) + m_strFileName;


		}
	}
	else
		strRet =strInput + m_strFileName;
	if (!strRet.IsEmpty())
	{
		MD5 md;
		md.update(strRet.GetBuffer());
		strRet = md.toString().c_str();
	}

	return strRet;
}
BOOL MyVerify::GetLocoalIPandMac(MYVECTOR &strMac)
{
	CStringA myIP, myMAC;
	bool bNetReady = false;
	ULONG outBufLen = 0;
	DWORD dwRetVal = 0;

	PIP_ADAPTER_INFO pAadpterInfo;
	PIP_ADAPTER_INFO pAadpterInfoTmp = NULL;



	pAadpterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GMEM_ZEROINIT, sizeof(IP_ADAPTER_INFO));

	if (GetAdaptersInfo(pAadpterInfo, &outBufLen) != ERROR_SUCCESS)
	{
		GlobalFree(pAadpterInfo);
		pAadpterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GMEM_ZEROINIT, outBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo(pAadpterInfo, &outBufLen)) == NO_ERROR)
	{
		pAadpterInfoTmp = pAadpterInfo;
		myIP = "";
		CStringA strTemp;
		while (pAadpterInfoTmp)
		{
			myMAC = ""; //����
			//if (pAadpterInfoTmp->Type == MIB_IF_TYPE_ETHERNET)
			{
				bNetReady = true;
				for (DWORD i = 0; i < pAadpterInfoTmp->AddressLength; i++)
				{
					if (i < pAadpterInfoTmp->AddressLength - 1)
						strTemp.Format("%02X-", pAadpterInfoTmp->Address[i]);
					else
						strTemp.Format("%02X", pAadpterInfoTmp->Address[i]);
					myMAC += strTemp;
				}
					
// 				myMAC.Format(("%02X-%02X-%02X-%02X-%02X-%02X"),
// 					pAadpterInfoTmp->Address[0],
// 					pAadpterInfoTmp->Address[1],
// 					pAadpterInfoTmp->Address[2],
// 					pAadpterInfoTmp->Address[3],
// 					pAadpterInfoTmp->Address[4],
// 					pAadpterInfoTmp->Address[5]);
				myMAC.Trim();
				strMac.push_back(myMAC);
			}
			pAadpterInfoTmp = pAadpterInfoTmp->Next;
		}
	}
	GlobalFree(pAadpterInfo);

	return bNetReady;
}


CStringA MyVerify::ReturnPcMac()
{
	CStringA strRet;
	if (GetLocoalIPandMac(m_strMac))
	{
		strRet = *(m_strMac.begin()) ;
	}
	return strRet;
}

BOOL MyVerify::VerityExe()
{
	BOOL retBool = FALSE;
	//1 ��ȡ���� ��̫�� ������ ����mac
	
	if (!GetLocoalIPandMac(m_strMac))
	{
		MessageBox(NULL, _T("��ȡ������̫��������macʧ��,��������ʧ��!"), _T("������ʾ"), MB_OK);
		return FALSE;
	}
	TCHAR bufPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, bufPath, MAX_PATH);

	//1. �鿴��Ȩ�ļ��Ƿ����
	CString strFile = bufPath;
	int index = strFile.ReverseFind('\\');
	strFile = strFile.Left(index + 1)+CpublicFun::AscToUnicode(m_strFileName);
	CFile file;
	if (file.Open(strFile, CFile::modeRead))
	{
		DWORD len = file.GetLength();
		if (len > 0)
		{
			CFileException ex;
			char* buf = new char[len + 1];
			memset(buf, 0, len + 1);
			file.Read(buf, len);
			
			CStringA strMac(buf);
			delete [] buf;
			strMac.Trim();
			MYVECTOR::iterator begin = m_strMac.begin(), end = m_strMac.end();
			MD5 md;
			
			//���� �Ƿ� ע��
			CStringA strTemp;
			for (; begin != end; ++begin)
			{
				strTemp = *begin + m_strFileName;
				md.update(strTemp.GetBuffer());
				strTemp = CString(md.toString().c_str());
				if (strTemp == strMac)
				{
					retBool = TRUE;
					break;
				}
				md.reset();
			}

		}

		file.Close();
	}
	return retBool;
}

//���� ע����
//�ɹ� ����true ��д���ļ�
//ʧ�� ����false ��д�ļ�
BOOL  MyVerify::WriteAndVerityExe(CStringA const& strVerity)
{
	BOOL retBool = FALSE;
	if (!GetLocoalIPandMac(m_strMac))
	{
		MessageBox(NULL, _T("��ȡ������̫��������macʧ��,��������ʧ��!"), _T("������ʾ"), MB_OK);
		return FALSE;
	}

	CStringA strMac = strVerity;
	strMac.Trim();
	MYVECTOR::iterator begin = m_strMac.begin(), end = m_strMac.end();
	MD5 md;

	//���� �Ƿ� ע��
	CStringA strTemp;
	for (; begin != end; ++begin)
	{
		strTemp = *begin + m_strFileName;
		md.update(strTemp.GetBuffer());
		if (CStringA(md.toString().c_str()) == strMac)
		{
			retBool = TRUE;
			break;
		}
		md.reset();
	}
		if (FALSE == retBool)
		{
			return retBool;
		}


		TCHAR bufPath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, bufPath, MAX_PATH);

		//1. �鿴��Ȩ�ļ��Ƿ����
		CString strFile = bufPath;
		int index = strFile.ReverseFind('\\');
		strFile = strFile.Left(index + 1) + CpublicFun::AscToUnicode(m_strFileName);
		CFile file;
		if (file.Open(strFile, CFile::modeCreate|CFile::modeReadWrite))
		{
			
			file.Write(strMac, strMac.GetLength());
			file.Close();
		}
		
		return retBool;
}