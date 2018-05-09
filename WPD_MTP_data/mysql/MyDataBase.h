#pragma once
#include <string>    
#include <vector>  
#include "include/mysql.h"
// ����MySQL������Ϣ  
typedef CStringA MYCHAR;
typedef struct
{
	MYCHAR server;
	MYCHAR user;
	MYCHAR password;
	MYCHAR database;
	int port;
}MySQLConInfo;


class CMyDataBase
{
public:

	~CMyDataBase();

	void SetMySQLConInfo(char* server, char* username, char* password, char* database, int port);// ����������Ϣ  
	bool Open();  // ������  
	void Close(); // �ر�����  

	bool Select(const std::string& Querystr, std::vector<std::vector<std::string> >& data);      // ��ȡ����  
	bool Query(const std::string& Querystr);     // ��������  ִ�ж��� 
	bool OneQuery(const std::string& Querystr); //ִ�е���
	void ErrorIntoMySQL();       // ������Ϣ  
	const char* GetErrorInfo();  //��ȡ������Ϣ
public:
	int ErrorNum;                // �������    
	const char* ErrorInfo;       // ������ʾ    

private:
	MySQLConInfo MysqlConInfo;   // ������Ϣ  
	MYSQL MysqlInstance;         // MySQL����  
	MYSQL_RES *Result;           // ���ڴ�Ž�� 
private:
	CMyDataBase();
	static CMyDataBase* m_singletion;
public:
	static CMyDataBase* GetInstance();
	/*
	��ʼ�����ݿ�:��ȡ�����ļ��е���Ϣ ��½ Զ�����ݿ�
	strPath: �����ļ��ľ���·��
	*/
	bool InitMyDataBase(MySQLConInfo const& conInfo);
};

