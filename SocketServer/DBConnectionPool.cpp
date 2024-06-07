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


//커넥션 스트링은 어떤 db랑 어떤 조건으로 연결할지 알려준다.
bool DBConnectionPool::Connect(int connectionCount, const WCHAR* conString)
{
	//락을 걸어주는디??
	//연결을 맺어주는 부분.
	//처음에 서버가 뜰때 딱 한번만 호출한다.

	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_henv) != SQL_SUCCESS) {
		return false;
	}

	if (::SQLSetEnvAttr(_henv, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0)!=SQL_SUCCESS) {
		return false;
	}

	//커넥션연결 수만큼 연결을 맺어줄거다. 
	//사실 위에는 걍 그저그런일 밑에부터가 중요한일이다.
	//[연결] 우리는 연결을 통해 db커넥션객체를 통해 db요청을 내린다. 
	// 근데 그 요청을 한 스레드에서만 한다고 보장할 수 없다.
	for (int i = 0; i < connectionCount; ++i) {

		//루키스는 xnew 사용했는게 그게 모임?
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
	//WRITE_LOCK;
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
	//WRITE_LOCK;

	if (_connections.empty())
		return nullptr;

	DBConnection* connection = _connections.back();
	_connections.pop_back();
	return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
	//WRITE_LOCK;
	_connections.push_back(connection);
}