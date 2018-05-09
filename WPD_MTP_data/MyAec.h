#pragma once
class MyAec
{
public:
	MyAec();
	~MyAec();
	std::string ECB_AESEncryptStr(std::string sKey, const char *plainText);
	std::string ECB_AESDecryptStr(std::string sKey, const char *cipherText);

};

