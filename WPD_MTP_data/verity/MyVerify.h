#pragma once
/*
校验类
提供接口:

*/
#include <vector>

typedef  std::vector<CStringA>  MYVECTOR;
class MyVerify
{
private:
	MyVerify();
	

private:
	//保存本机mac地址
	MYVECTOR m_strMac;
	BOOL GetLocoalIPandMac(MYVECTOR &strMac);
	CStringA m_strFileName;
public:
	/*获取本地mac地址*/
	CStringA ReturnPcMac();
	CStringA GetEncryptStr(CStringA const&); //获取加密后的数据
	BOOL  VerityExe();
	BOOL  WriteAndVerityExe(CStringA const& strVerity);
	~MyVerify();
	static MyVerify* m_singleton;
	static MyVerify* GetInstance();
	//md5 加密
	CStringA EncryptStrMD5(CStringA const&);
};

