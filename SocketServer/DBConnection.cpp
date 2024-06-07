#include "stdafx.h"
#include "DBConnection.h"

DBConnection::DBConnection()
{
	_connection = SQL_NULL_HANDLE;
}

DBConnection::~DBConnection()
{
}

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connString)
{
	if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection)!=SQL_SUCCESS) {
		return false;
	}

	//Ŀ�ؼ� string�� �̿��ؼ� ���������� db������ �����Ѵ�.
	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connString);

	//����� ����� ����
	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0;

	
	//Ŀ�ؼ��� �̿��ؼ� db�� �����Ѵ�
	SQLRETURN ret = SQLConnect(_connection, (SQLWCHAR*)connString, SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS) {
		return false;
	}

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);


}

void DBConnection::clear()
{
	//����ϰ� �����ϱ����� 
	if (_connection != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}

	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}

}

//�� �Լ��� ������ �߿��� �Լ��̴�.
//������ ���ڷ� ������ �����ϴ� �Լ��̴�.
bool DBConnection::Excute(const WCHAR* query)
{
	SQLRETURN ret = ::SQLExecDirect(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;

}

//�����͸� �ܾ�ö� ����ϴ� �Լ�
bool DBConnection::Fetch()
{
	//����� �޾ƿ��°Ͱ� ���õ� �Լ�
	//ret�� �����̸� �޾ƿ� �����Ͱ� �ִٴ� ���̴�
	SQLRETURN ret = ::SQLFetch(_statement);


	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		//��û�Ѱſ� ���� �߸��� ����� �߻���������
		HandleError(ret);
		return false;
	default:
		return true;
	}

}

int DBConnection::GetRowCount()
{
	//�����Ͱ� � �ִ����� ��ȯ�ϴ� �Լ�
	SQLLEN count = 0;
	//_statement�� ���ڸ� �ѱ涧�� �� �� ������ �Ųٷ� �ܾ�� ��������� �����Ҷ��� ����Ѵ�
	SQLRETURN ret = ::SQLRowCount(_statement, OUT &count);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return static_cast<int>(count);

	return -1;
}

void DBConnection::Unbind()
{
	//���ڸ� �޾ƿ��ų� �Ѱ��ֱ� ���� ������ ����Ǿ��ִ� ���� �ѹ� �����ִ°���
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);

}

bool DBConnection::BindParameter(SQLUSMALLINT paramIdx, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
{
	//���ڸ� �Ѱ��ֱ�

	//SQLBindParameter
	//�� �Լ����� _statement�� ���ؼ� ���° ����paramIdx�� ����cType���� �츮sqlType�� �����ϰ�������� ä���ִ°���
	//���������� �����Ͱ� �ִ� ptr�� �ѱ�°���.
	SQLRETURN ret = ::SQLBindParameter(_statement, paramIdx, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

bool DBConnection::BindCol(SQLUSMALLINT colummIdx, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
	//���ڸ� �޾ƿ���

	//ȯ���ڵ��� �Ѱ��ְ� ���������� �ǳ��ش�
	SQLRETURN ret = ::SQLBindCol(_statement, colummIdx, cType, value, len, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
	//�� �Լ��� ret�� ���� ������ ����ϴ� �ڵ�κ��� �ݺ������� �����ϱ⶧���� ���� �� �Լ���

	if (ret == SQL_SUCCESS)
		return;

	//�� �޾��ִ� ���ڵ���, �ܿ��ʿ���� �ѹ��� �����ָ�Ǵ°���
	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeErr = 0;
	SQLWCHAR errMsg[MAX_PATH] = { 0 };
	SQLSMALLINT msgLen = 0;
	SQLRETURN errorRet = 0;

	while (true)
	{
		//��Ȯ�� �����޽����� �������ش�.
		errorRet = ::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,			
			sqlState,
			OUT & nativeErr,
			errMsg,
			_countof(errMsg),
			OUT & msgLen
		);

		if (errorRet == SQL_NO_DATA)
			break;

		if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
			break;

		// TODO : Log
		//���� ���̺꿡���� �ܼ��� ���ߴܰ迡���� üũ�ϴ°���
		//���̺귯�� �̿��ؼ� file�� log�����Ѵ�.
		std::wcout.imbue(std::locale("kor"));
		std::wcout << errMsg << std::endl;

		index++;
	}

}

bool DBConnection::BindParam(int paramIdx, bool* value, SQLLEN* index)
{
	return BindParameter(paramIdx, SQL_C_TINYINT, SQL_C_TINYINT, sizeof(bool), value, index);
}

bool DBConnection::BindParam(int paramIdx, float* value, SQLLEN* index)
{
	return BindParameter(paramIdx, SQL_C_FLOAT, SQL_REAL, 0, value, index);
}

bool DBConnection::BindParam(int paramIdx, double* value, SQLLEN* index)
{
	return BindParameter(paramIdx, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
}

bool DBConnection::BindParam(int paramIdx, int* value, SQLLEN* index)
{
	return BindParameter(paramIdx, SQL_C_LONG, SQL_INTEGER, sizeof(int), value, index);
}

bool DBConnection::BindParam(int paramIdx, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindParameter(paramIdx, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindParam(int paramIdx, const WCHAR* value, SQLLEN* index)
{
	SQLULEN size = static_cast<SQLULEN>((::wcslen(value) + 1) * 2);
	*index = SQL_NTSL;

	if (size > 4000) {

		return BindParameter(paramIdx, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)value, index);
	}
	else {

		return BindParameter(paramIdx, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)value, index);
	}
}

bool DBConnection::BindParam(int paramIdx, const char* value, SQLLEN* index)
{

	SQLULEN size = static_cast<SQLULEN>((::strlen(value) + 1) * 2);
	*index = SQL_NTSL;

	if (size > 4000) {

		return BindParameter(paramIdx, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)value, index);
	}
	else {

		return BindParameter(paramIdx, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)value, index);
	}
}

bool DBConnection::BindParam(int paramIdx, const BYTE* value, int size, SQLLEN* index)
{
	if (value == nullptr)
	{
		*index = SQL_NULL_DATA;
		size = 1;
	}
	else
		*index = size;

	if (size > 8000) {
		return BindParameter(paramIdx, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)value, index);
	}
	else {
		return BindParameter(paramIdx, SQL_C_BINARY, SQL_BINARY, size, (BYTE*)value, index);
	}
}

bool DBConnection::BindCol(int columnIdx, bool* value, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_TINYINT, sizeof(bool), value, index);
}

bool DBConnection::BindCol(int columnIdx, float* value, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_FLOAT, sizeof(float), value, index);
}

bool DBConnection::BindCol(int columnIdx, double* value, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_DOUBLE, sizeof(double), value, index);
}

bool DBConnection::BindCol(int columnIdx, int* value, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_LONG, sizeof(int), value, index);
}

bool DBConnection::BindCol(int columnIdx, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_TYPE_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindCol(int columnIdx, WCHAR* str, int size, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_WCHAR, size, str, index);
}

bool DBConnection::BindCol(int columnIdx, char* str, int size, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_C_CHAR, size, str, index);
}

bool DBConnection::BindCol(int columnIdx, BYTE* bin, int size, SQLLEN* index)
{
	return BindCol(columnIdx, SQL_BINARY, size, bin, index);
}

