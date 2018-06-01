#include "..\\stdafx.h"
#include "MyDataBase.h"

 CMyDataBase*  CMyDataBase::m_singletion;

 CMyDataBase*  CMyDataBase::GetInstance()
 {
	 if (nullptr == m_singletion)
	 {
		 m_singletion = new CMyDataBase();
	 }
	 return m_singletion;
}
CMyDataBase::CMyDataBase():ErrorNum(0), ErrorInfo("ok")
{
	m_singletion = nullptr;
	m_isConnect = FALSE;
}
/*
��ʼ�����ݿ�:��ȡ�����ļ��е���Ϣ ��½ Զ�����ݿ�
strPath: �����ļ��ľ���·��
*/
bool CMyDataBase::InitMyDataBase(MySQLConInfo const& conInfo)
{

	MysqlConInfo = conInfo;
	
	mysql_library_init(0, NULL, NULL);

	mysql_init(&MysqlInstance);
	// �����ַ����������޷���������  
	mysql_options(&MysqlInstance, MYSQL_SET_CHARSET_NAME, "gbk");

	m_isConnect = Open();

	return m_isConnect;
}

CMyDataBase::~CMyDataBase()
{
	mysql_server_end();
}
   

// ����������Ϣ  
void CMyDataBase::SetMySQLConInfo(char* server, char* username, char* password, char* database, int port)
{
	MysqlConInfo.server = server;
	MysqlConInfo.user = username;
	MysqlConInfo.password = password;
	MysqlConInfo.database = database;
	MysqlConInfo.port = port;
}

// ������  
bool CMyDataBase::Open()
{
	if (mysql_real_connect(&MysqlInstance, MysqlConInfo.server, MysqlConInfo.user,
		MysqlConInfo.password, MysqlConInfo.database, MysqlConInfo.port, 0, 0) != NULL)
	{
		return true;
	}
	else
	{
		ErrorIntoMySQL();
		return false;
	}
}

// �Ͽ�����  
void CMyDataBase::Close()
{
	mysql_close(&MysqlInstance);
}

//��ȡ����  
bool CMyDataBase::Select(const std::string& Querystr, std::vector<std::vector<std::string> >& data)
{

	if (0 != mysql_query(&MysqlInstance, Querystr.c_str()))
	{
		ErrorIntoMySQL();
		return false;
	}

	Result = mysql_store_result(&MysqlInstance);

	// ������  
	int row = mysql_num_rows(Result);
	int field = mysql_num_fields(Result);

	MYSQL_ROW line = NULL;
	line = mysql_fetch_row(Result);

	int j = 0;
	std::string temp;
	std::vector<std::vector<std::string> >().swap(data);
	while (NULL != line)
	{
		std::vector<std::string> linedata;
		for (int i = 0; i < field; i++)
		{
			if (line[i])
			{
				temp = line[i];
				linedata.push_back(temp);
			}
			else
			{
				temp = "";
				linedata.push_back(temp);
			}
		}
		line = mysql_fetch_row(Result);
		data.push_back(linedata);
	}
	return true;
}

// ��������  
bool CMyDataBase::Query(const std::string& Querystr)
{
	//һ��ִ�ж������
	mysql_set_server_option(&MysqlInstance, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	int ret = mysql_query(&MysqlInstance, Querystr.c_str());
	mysql_set_server_option(&MysqlInstance, MYSQL_OPTION_MULTI_STATEMENTS_OFF);
	
	do
	{
		Result = mysql_store_result(&MysqlInstance);
		mysql_free_result(Result);
	} while (!mysql_next_result(&MysqlInstance));
	if (0 == ret)
	{
		return true;
	}
	ErrorIntoMySQL();
	return false;
}

// ��������  
bool CMyDataBase::OneQuery(const std::string& Querystr)
{
	//һ��ִ�ж������
	
	int ret = mysql_query(&MysqlInstance, Querystr.c_str());

	if (0 == ret)
	{
		return true;
	}
	ErrorIntoMySQL();
	return false;
}

//������Ϣ  
void CMyDataBase::ErrorIntoMySQL()
{
	ErrorNum = mysql_errno(&MysqlInstance);
	ErrorInfo = mysql_error(&MysqlInstance);
}
//��ȡ������Ϣ
const char* CMyDataBase::GetErrorInfo()
{
	return ErrorInfo;
}
