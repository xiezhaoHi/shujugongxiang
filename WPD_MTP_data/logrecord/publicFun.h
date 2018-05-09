#pragma once
#include <vector>

class CpublicFun
{
public:
	CpublicFun();
	~CpublicFun();
private:

public:

	static CStringA GBKToUTF8(CStringA const& strGBK)
	{
		
		WCHAR * str1;
		int n = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
		str1 = new WCHAR[n];
		MultiByteToWideChar(CP_ACP, 0, strGBK, -1, str1, n);

		n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
		char * str2 = new char[n];
		WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
		CStringA strRet(str2);
		
		delete[]str1;
		str1 = NULL;
		delete[]str2;
		str2 = NULL;
		return strRet;
	}

	static CStringA UnicodeToUTF8(CStringW const& strGBK)
	{
		int n = WideCharToMultiByte(CP_UTF8, 0, strGBK, -1, NULL, 0, NULL, NULL);
		char * str2 = new char [n];
		WideCharToMultiByte(CP_UTF8, 0, strGBK, -1, str2, n, NULL, NULL);
		CStringA strRet(str2);

		delete[]str2;
		str2 = NULL;
		return strRet;
	}


	static CStringA UnicodeToAsc(CStringW const& strUnicode)
	{
		int iTextLen=WideCharToMultiByte(CP_ACP,0,strUnicode,-1,NULL,0,NULL,0);//确定strText中的CString转换为ASCII后，所需的字节数
		char *lText = new char[iTextLen];
		memset(lText,0,iTextLen*sizeof(char));
		WideCharToMultiByte(CP_ACP,0,strUnicode,-1,lText,iTextLen,NULL,0);  // 将strText中的字符串全部转换成ASCII，并将其保存在lText开辟的字符空间中
		CStringA strRet(lText);
		delete lText;
		return strRet;
	}

	static CStringW AscToUnicode(CStringA const& strAsc)
	{

		int nLen = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, strAsc, -1, NULL, 0 );
		if (nLen == 0)
		{
			return NULL;
		}
		wchar_t* pResult = new wchar_t[nLen];
		memset(pResult,0,nLen*sizeof(wchar_t));

		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, strAsc, -1, pResult, nLen );

		CStringW strRet(pResult);
		delete pResult;

		return strRet;
	}
	
	static CString UTF8ToUnicode(const char* UTF8)
	{
		DWORD dwUnicodeLen;        //转换后Unicode的长度
		wchar_t *pwText;            //保存Unicode的指
		CString strUnicode;        //返回值
		//获得转换后的长度，并分配内存
		dwUnicodeLen = MultiByteToWideChar(CP_UTF8,0,UTF8,-1,NULL,0);
		pwText = new wchar_t[dwUnicodeLen];
		if (!pwText)
		{
			return strUnicode;
		}
		//转为Unicode
		MultiByteToWideChar(CP_UTF8,0,UTF8,-1,pwText,dwUnicodeLen);
		//转为CString
		strUnicode.Format(_T("%s"),pwText);
		//清除内存
		delete []pwText;
		//返回转换好的Unicode字串
		return strUnicode;

	}


	static CStringA Convert(CStringA str, int sourceCodepage, int targetCodepage)
	{
		int len=str.GetLength();
		int unicodeLen=MultiByteToWideChar(sourceCodepage,0,str,-1,NULL,0);
		wchar_t* pUnicode;
		pUnicode=new wchar_t[unicodeLen+1];
		memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));
		MultiByteToWideChar(sourceCodepage,0,str,-1,(LPWSTR)pUnicode,unicodeLen);
		BYTE * pTargetData = NULL;
		int targetLen=WideCharToMultiByte(targetCodepage,0,(LPWSTR)pUnicode,-1,(char *)pTargetData,0,NULL,NULL);
		pTargetData=new BYTE[targetLen+1];
		memset(pTargetData,0,targetLen+1);
		WideCharToMultiByte(targetCodepage,0,(LPWSTR)pUnicode,-1,(char *)pTargetData,targetLen,NULL,NULL);
		CStringA rt;
		rt.Format("%s",pTargetData);
		delete pUnicode;
		delete pTargetData;
		return rt;
	}


	static CString GetDirectory()
	{	
		BROWSEINFO bi;	
		TCHAR name[MAX_PATH];	
		ZeroMemory(&bi,sizeof(BROWSEINFO));	
		bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();	
		bi.pszDisplayName = name;	
		bi.lpszTitle = _T("选择导出文件夹目录");	
		bi.ulFlags = BIF_RETURNFSANCESTORS;	
		LPITEMIDLIST idl = SHBrowseForFolder(&bi);	
		if(idl == NULL)		
			return _T("");	
		CString strDirectoryPath;	
		SHGetPathFromIDList(idl, strDirectoryPath.GetBuffer(MAX_PATH));	
		strDirectoryPath.ReleaseBuffer();	
		if(strDirectoryPath.IsEmpty())		
			return _T("");	
		if(strDirectoryPath.Right(1)!=_T("\\"))
			strDirectoryPath+=_T("\\");		
		return strDirectoryPath;
	}

	static void TraverseDir(CString& dir, std::vector<CString>& vec)
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

	static BOOL ClearAllFile(CString & strDir,CString & strErr)
	{
		std::vector<CString> vec;
		CTime curTime = CTime::GetTickCount();
		TraverseDir(strDir,vec);
		for(std::vector<CString>::const_iterator it = vec.begin(); it < vec.end(); ++it)
		{

			{
				DeleteFile(*it);
			}
		}
		return TRUE;
	}


	static CStringA UTF8ToASCII(const char* UTF8)
	{
		return UnicodeToAsc(UTF8ToUnicode(UTF8));
	}


};


