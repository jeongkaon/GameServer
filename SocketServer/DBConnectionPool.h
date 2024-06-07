#pragma once

#include "DBConnection.h"


//전역으로 딱 한개만 들고있을것이다
//필요한 애가 꺼내서 쓸거임.
class DBConnectionPool
{
	
	SQLHENV _henv;		//환경을 담당하는 핸들
	std::vector<DBConnection*> _connections;

public:
	DBConnectionPool();
	~DBConnectionPool();

	//몇개의 커넥션을 생성할지, 커넥션 스트링을 인자로 받겠다.
	bool Connect(int connectionCount, const WCHAR* conString);
	void Clear();

	//풀에서 실질적으로 꺼내는거랑 반납하는함수
	DBConnection* Pop();
	void Push(DBConnection* conn);
};

