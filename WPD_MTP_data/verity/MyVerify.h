#pragma once
/*
У����
�ṩ�ӿ�:

*/
#include <vector>

typedef  std::vector<CStringA>  MYVECTOR;
class MyVerify
{
private:
	MyVerify();
	

private:
	//���汾��mac��ַ
	MYVECTOR m_strMac;
	BOOL GetLocoalIPandMac(MYVECTOR &strMac);
	CStringA m_strFileName;
public:
	/*��ȡ����mac��ַ*/
	CStringA ReturnPcMac();
	CStringA GetEncryptStr(CStringA const&); //��ȡ���ܺ������
	BOOL  VerityExe();
	BOOL  WriteAndVerityExe(CStringA const& strVerity);
	~MyVerify();
	static MyVerify* m_singleton;
	static MyVerify* GetInstance();
	//md5 ����
	CStringA EncryptStrMD5(CStringA const&);
};

