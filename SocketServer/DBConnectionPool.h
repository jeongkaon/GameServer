#pragma once

#include "DBConnection.h"


class DBConnectionPool
{
	
	SQLHENV _henv;		
	std::vector<DBConnection*> _connections;

public:
	DBConnectionPool();
	~DBConnectionPool();

	bool Connect(int connectionCount, const WCHAR* conString);
	void Clear();

	DBConnection* Pop();
	void Push(DBConnection* conn);
};

