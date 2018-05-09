#ifndef ISQLITE_H
#define ISQLITE_H
//#include "stdafx.h"
#include "sqlite3.h"					//����sqlite3.h ͷ�ļ� ��ͬ��Ŀ ����·����ͬ
	//����SQLITE3.LIB ͷ�ļ� ��ͬ��Ŀ ����·����ͬ
using namespace std;
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SQLITE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SQLITE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#ifdef  SQLITE_EXPORTS
#define CSQLITE_API __declspec(dllexport)
#else
#define CSQLITE_API __declspec(dllimport) 
#endif
//////////////////////////////////////////////////////////////////////////
// sqlite3 �������ݿ�
//////////////////////////////////////////////////////////////////////////
class ISQLite
{
public:
	//************************************
	// Method:    OpenDataBase �����ݿ�
	// Parameter: const char * sz_db3Path ���ݿ�·��
	// Parameter: BOOL synchronous �Ƿ������١���дͬ��
	//************************************
	virtual BOOL	OpenDataBase( const char* sz_db3Path,BOOL  synchronous = FALSE ) = 0;
	//************************************
	// Method:    CreateTable ������
	// Parameter: const char * sz_creattableSql �������sql���  ����create table test (id int, name text)
	//************************************
	virtual BOOL	CreateTable( const char* sz_creattableSql ) = 0;
	//************************************
	// Method:    InsertData  ����в�������
	// Parameter: const char * sz_insertSql  ��������sql ����INSERT INTO test VALUES(?,?)
	//************************************
	virtual BOOL	InsertData( const char* sz_insertSql ) = 0;
	//************************************
	// Method:    SelectData  ��ѯ��������
	// Parameter: const char * sz_selectSql ��ѯsql ����select * from test where ...
	// Parameter: int 
	// Parameter: * Callback �ص�����
	// Parameter: void * notUsed
	// Parameter: int argc ��ѯ�����ݵ�����
	// Parameter: char * * argv �ֶ�ֵ
	// Parameter: char * * azColName �ֶ���
	//************************************
	virtual BOOL	SelectData( const char* sz_selectSql,
		int (*Callback)(void *notUsed, int argc, char **argv, char **azColName) = NULL,void* vid = NULL) = 0;
	//************************************
	// Method:    UpdataData ��������
	// Parameter: const char * sz_updataSql ��������sql ����update test set name='%s', birthday='d-d-d d:d:d' where id=%d;
	//************************************
	virtual BOOL	UpdataData( const char* sz_updataSql ) = 0;
	//************************************
	// Method:    ReplaceData  �������� ������� �������
	// Parameter: int id ����id ����  replace into TESTTABLE values (%d,'%s','d-d-d d:d:d')
	//************************************
	virtual BOOL    ReplaceData(const char* sz_ReplaceSql) = 0;
	//************************************
	// Method:    DeleteData  ɾ������
	// Parameter: const char * sz_deleteSql ɾ������sql ����DELETE FROM test
	//************************************
	virtual BOOL	DeleteData( const char* sz_deleteSql ) = 0;
	//************************************
	// Method:    Droptable ɾ����
	// Parameter: const char * sz_droptableSql ɾ����sql  ����DROP TABLE TEST
	//************************************
	virtual BOOL	Droptable(const char* sz_droptableSql ) = 0;
	//************************************
	// Method:    CloseDataBase �ر����ݿ�
	//************************************
	virtual BOOL	CloseDataBase() = 0;
	//************************************
	// Method:    QuickInsertData  ���ٲ��� ʹ�ýṹ�󶨷�ʽ
	// Parameter: const char * insertSql ����sql ����INSERT INTO test VALUES(?,?)  ��Ӧ��ֵ�ã���ʾ
	// Parameter: CStringArray * const & strArr_in ���󶨵�CStringArray*
	// Parameter: int const & count ����CStringArray* �ĸ���
	// Parameter: * sqlite3_bind �ص����� �Լ������CStringArray�е��ַ���
	// Parameter: sqlite3_stmt * �󶨵Ľṹ
	// Parameter: CStringArray const & strArr_in ���󶨵�CStringArray
	//************************************
//	virtual BOOL	QuickInsertData(const char* insertSql,CStringArray* const& strArr_in,int const& count,
//										BOOL (*sqlite3_bind)(sqlite3_stmt*,CStringArray const& strArr_in)) = 0;
	virtual BOOL	QuickInsertData(const char* insertSql,CStringArray** strArr_in,int const& count,int const& number,
		BOOL (*sqlite3_bind)(sqlite3_stmt*,CStringArray* const& strArr_in,int const& number)) = 0;


	virtual BOOL	QuickDeletData(const char* insertSql,CStringArray** strArr_in,int const& count,int const& number,
		BOOL (*sqlite3_bind)(sqlite3_stmt*,CStringArray* const& strArr_in,int const& number)) = 0;

	virtual BOOL	QuickSelectData(const char* insertSql,CStringArray** strArr_in,int& count,void* vid = NULL,
		BOOL (*select_bind)( void* vid,sqlite3_stmt* stmt,CStringArray& strArr_in,int const& nColumn )= NULL) = 0;
	//************************************
	// Method:    DeleteThis ɾ���Լ�����  ɾ��dll�ļ���ʽʹ��CreateCSQLite���صĶ���
	//************************************
	//BEGIN TRANSACTION
	virtual BOOL	BeginTransaction() = 0;
	//commit
	virtual BOOL	Commit() = 0;
	//
	virtual void	CreateIndex(const char* CreateIndxeSql) = 0;
	virtual void    DeleteThis() = 0;
};

extern "C" CSQLITE_API ISQLite* CreateCSQLite(const char* InvType,const char* pwd );
#endif


/*
// ʹ��QuickInsertData���� ������ʵ��
BOOL CSQLite::sqlite3_bind( sqlite3_stmt* stmt,CStringArray const& strArr_in )
{
	try
	{
		//�ڰ�ʱ��������ı�������ֵ��1��
		sqlite3_bind_int(stmt, 1, 1);   //��int����
		//ʹ���˷�UNICODE ����
		//cstring to char* ����
		sqlite3_bind_text(stmt, 2, strArr_in.GetAt(1), strlen(strArr_in.GetAt(1)), SQLITE_TRANSIENT);  //��char*����
		return TRUE;
	}
	catch ( ... )
	{
		return FALSE;
	}
}
//��ѯ�ص�����ʹ��ʵ��
int CSQLiteDlg::SelectCallback(void *notUsed, int argc, char **argv, char **azColName)
{
	for (int i = 0 ; i < argc ; i++)
	{
		_cprintf("%s = %s  ", azColName[i], (argv[i] ? argv[i] : "NULL"));
		if (i != argc -1)
		{
			_cprintf(", ");
			_cprintf("\n");
		}
	}
	return 0;
}
*/