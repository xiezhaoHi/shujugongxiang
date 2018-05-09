#include "stdafx.h"
#include "MyAec.h"
#include <aes.h>  
#include <Hex.h>      // StreamTransformationFilter  
#include <modes.h>    // CFB_Mode  
#include <iostream>   // std:cerr    
#include <sstream>   // std::stringstream    
#include <string>  
using namespace CryptoPP;
unsigned char key[] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08, 0x01,0x02, 0x03,0x04,0x05,0x06,0x07,0x08 };//AES::DEFAULT_KEYLENGTH  
unsigned char iv[] = { 0x01,0x02,0x03,0x03,0x03,0x03,0x03,0x03, 0x03,0x03, 0x01,0x02,0x03,0x03,0x03,0x03 };
int keysize = 16;
MyAec::MyAec()
{
}


MyAec::~MyAec()
{
}
std::string MyAec::ECB_AESEncryptStr(std::string sKey, const char *plainText)
{

	std::string outstr;

	//ÃÓkey    
	SecByteBlock key(AES::MAX_KEYLENGTH);
	memset(key, 0x30, key.size());
	sKey.size() <= AES::MAX_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::MAX_KEYLENGTH);


	AES::Encryption aesEncryption((unsigned char *)key, AES::MAX_KEYLENGTH);

	ECB_Mode_ExternalCipher::Encryption ecbEncryption(aesEncryption);
	StreamTransformationFilter ecbEncryptor(ecbEncryption, new HexEncoder(new StringSink(outstr)));
	ecbEncryptor.Put((unsigned char *)plainText, strlen(plainText));
	ecbEncryptor.MessageEnd();

	return outstr;
}

std::string MyAec::ECB_AESDecryptStr(std::string sKey, const char *cipherText)
{
	
	try
	{
		std::string outstr;

		//ÃÓkey    
		SecByteBlock key(AES::MAX_KEYLENGTH);
		memset(key, 0x30, key.size());
		sKey.size() <= AES::MAX_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::MAX_KEYLENGTH);

		ECB_Mode<AES >::Decryption ecbDecryption((unsigned char *)key, AES::MAX_KEYLENGTH);

		HexDecoder decryptor(new StreamTransformationFilter(ecbDecryption, new StringSink(outstr)));
		decryptor.Put((unsigned char *)cipherText, strlen(cipherText));
		decryptor.MessageEnd();

	}
	catch (...) //“Ï≥£¥¶¿Ì
	{
		return "";
	}
}


