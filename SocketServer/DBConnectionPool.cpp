#include "stdafx.h"
#include "DBConnectionPool.h"

DBConnectionPool::DBConnectionPool()
{
	_henv = SQL_NULL_HANDLE;
}

DBConnectionPool::~DBConnectionPool()
{
	Clear();
}


bool DBConnectionPool::Connect(int connectionCount, const WCHAR* conString)
{

	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_henv) != SQL_SUCCESS) {
		return false;
	}

	if (::SQLSetEnvAttr(_henv, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0)!=SQL_SUCCESS) {
		return false;
	}

	for (int i = 0; i < connectionCount; ++i) {

		DBConnection* conn = new DBConnection();
		if (conn->Connect(_henv, conString) == false) {
			return false;
		}
		_connections.push_back(conn);
	}

	return true;
}

void DBConnectionPool::Clear()
{
	{
		
		if (_henv != SQL_NULL_HANDLE)
		{
			::SQLFreeHandle(SQL_HANDLE_ENV, _henv);
			_henv = SQL_NULL_HANDLE;
		}

		for (DBConnection* connection : _connections)
			delete(connection);

		_connections.clear();

	}
}

DBConnection* DBConnectionPool::Pop()
{

	if (_connections.empty())
		return nullptr;

	DBConnection* connection = _connections.back();
	_connections.pop_back();
	return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
	_connections.push_back(connection);
}