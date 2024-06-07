#pragma once
#include <sql.h>
#include <sqlext.h>


class DBConnection
{

	SQLHDBC _connection;		//db 커넥션을 담당하는 핸들
	SQLHSTMT _statement;		//상태를 관리한다.


public:
	DBConnection();
	~DBConnection();

	bool Connect(SQLHENV henv, const WCHAR* connString);
	void clear();

	//실제 쿼리를 실행한다.
	bool Excute(const WCHAR* query);
	bool Fetch();	//실행한 후 결과를 받아와서 처리해야할게 있는함수
	int  GetRowCount();
	void Unbind();



private:
	//이함수들은 내부적으로 사용할거임 
	//직접적으로 타입을 지정하는 부분들이 실수할 가능성이 높다


	//sql query를 실행할때, 인자들을 넘겨줘야하는 부분들이 자주발생
	//몇번째 파라미터인지 인자를 넘겨줄 인자가 여러개일수 있으니 순서를 지정해준다.
	bool BindParameter(SQLUSMALLINT paramIdx, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);

	//쿼리를 실행한 다음에 데이터를 거꾸로 긁어올때 
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

