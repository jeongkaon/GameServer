#pragma once
#include <sql.h>
#include <sqlext.h>


class DBConnection
{

	SQLHDBC _connection;		
	SQLHSTMT _statement;		


public:
	DBConnection();
	~DBConnection();

	bool Connect(SQLHENV henv, const WCHAR* connString);
	void clear();

	bool Excute(const WCHAR* query);
	bool Fetch();	//실행한 후 결과를 받아와서 처리해야할게 있는함수
	int  GetRowCount();
	void Unbind();



private:
	bool BindParameter(SQLUSMALLINT paramIdx, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);

	bool BindCol(SQLUSMALLINT colummIdx, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);
	void HandleError(SQLRETURN ret);

public:
	bool BindParam(int paramIdx, bool* value, SQLLEN* index);
	bool BindParam(int paramIdx, float* value, SQLLEN* index);
	bool BindParam(int paramIdx, double* value, SQLLEN* index);
	bool BindParam(int paramIdx, int* value, SQLLEN* index);
	bool BindParam(int paramIdx, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindParam(int paramIdx, const WCHAR* value, SQLLEN* index);
	bool BindParam(int paramIdx, const char* value, SQLLEN* index);
	bool BindParam(int paramIdx, const BYTE* value,int size, SQLLEN* index);

	bool BindCol(int columnIdx, bool* value, SQLLEN* index);
	bool BindCol(int columnIdx, float* value, SQLLEN* index);
	bool BindCol(int columnIdx, double* value, SQLLEN* index);
	bool BindCol(int columnIdx, int* value, SQLLEN* index);
	bool BindCol(int columnIdx, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindCol(int columnIdx, WCHAR* str, int size, SQLLEN* index);
	bool BindCol(int columnIdx, char* str, int size, SQLLEN* index);

	bool BindCol(int columnIdx, BYTE* bin, int size, SQLLEN* index);


};

