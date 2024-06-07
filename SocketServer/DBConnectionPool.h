#pragma once

#include "DBConnection.h"


//�������� �� �Ѱ��� ����������̴�
//�ʿ��� �ְ� ������ ������.
class DBConnectionPool
{
	
	SQLHENV _henv;		//ȯ���� ����ϴ� �ڵ�
	std::vector<DBConnection*> _connections;

public:
	DBConnectionPool();
	~DBConnectionPool();

	//��� Ŀ�ؼ��� ��������, Ŀ�ؼ� ��Ʈ���� ���ڷ� �ްڴ�.
	bool Connect(int connectionCount, const WCHAR* conString);
	void Clear();

	//Ǯ���� ���������� �����°Ŷ� �ݳ��ϴ��Լ�
	DBConnection* Pop();
	void Push(DBConnection* conn);
};

