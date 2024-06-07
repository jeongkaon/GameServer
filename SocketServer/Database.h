#pragma once

//db에 저장된대로 있어야함.
struct UserInfo 
{
	int pId;
	char Name[20];
	int x, y;
	short level;
	int exp;
	short HP;
};


class Database
{
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	SQLRETURN ret;


	SQLCHAR* OutConnStr = (SQLCHAR*)malloc(255);
	SQLSMALLINT* OutConnStrLen = (SQLSMALLINT*)malloc(255);


public:
	Database();
	~Database();

	bool connect();
	void disconnect();

	void LoadData();
	bool CheckIdInDatabase(const char* id);

};

