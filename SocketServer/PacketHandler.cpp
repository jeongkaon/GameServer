#include "stdafx.h"
#include "PacketHandler.h"
#include "SessionManager.h"
#include "MapManager.h"
#include "DBConnectionPool.h"

wchar_t* ConvertToWideChar(const char* str) {
	// 필요한 버퍼 크기 계산
	size_t len = strlen(str) + 1;
	wchar_t* buffer = new wchar_t[len];

	// 변환
	mbstowcs(buffer, str, len);

	return buffer;
}

void PacketHandler::Init(SessionManager* sessionMgr, MapManager* mapMgr, DBConnectionPool* dbConnPool)
{
	_sessionMgr = sessionMgr;
	_mapMgr = mapMgr;
	_dbConnPool = dbConnPool;

	_recvFuntionMap[(int)CS_REGIST] = &PacketHandler::ProcessRegistPacket;
	_recvFuntionMap[(int)CS_LOGIN] = &PacketHandler::ProcessLoginPacket;
	_recvFuntionMap[(int)CS_LOGIN_STRESS] = &PacketHandler::ProcessStressLoginPacket;
	_recvFuntionMap[(int)CS_CHOICE_CHARACTER] = &PacketHandler::ProcessChoiceCharactertPacket;
	_recvFuntionMap[(int)CS_MOVE] = &PacketHandler::ProcessMovePacket;
	_recvFuntionMap[(int)CS_CHAT] = &PacketHandler::ProcessChattingPacket;
	_recvFuntionMap[(int)CS_ATTACK] = &PacketHandler::ProcessAttackPacket;
	_recvFuntionMap[(int)CS_TELEPORT] = &PacketHandler::ProcessTeleportPacket;
	_recvFuntionMap[(int)CS_LOGOUT] = &PacketHandler::ProcessLogoutPacket;



}

void PacketHandler::ProcessRecvPacket(int id, char* buf, int copySize, int type)
{
	auto iter = _recvFuntionMap.find(type);
	if (iter != _recvFuntionMap.end())
	{
		(this->*(iter->second))(id, buf, copySize);
	}
	else {
		return;
	}
}


void PacketHandler::ProcessRegistPacket(int id, char* buf, int copySize)
{
	CS_REGIST_PACKET* p = reinterpret_cast<CS_REGIST_PACKET*>(buf);

}

void PacketHandler::ProcessLoginPacket(int id, char* buf, int copySize)
{
	CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(buf);
	
	GameData userData;
	strcpy_s(userData.user_name, p->name);
	userData.user_exp = 0;
	userData.user_level = 1;
	userData.user_hp = 100;
	userData.user_x = INIT_X_POS;
	userData.user_y = INIT_Y_POS;
	userData.user_visual =0;

	int res = CheckUserInDB(p->name, &userData);
	_sessionMgr->objects[id]->init(&userData);

	switch (res)
	{
	case NONE:
		_sessionMgr->LoginSession(id);
		break;
	case NOT_EXIST_IN_DB:
		AddUSerInDB(p->name);
		_sessionMgr->CharChoiceSession(id);
		break;
	case NOT_CHOICE_CHARACTER:
		_sessionMgr->CharChoiceSession(id);
		break;
	default:
		break;
	}

	std::cout << std::endl;

}

void PacketHandler::ProcessStressLoginPacket(int id, char* buf, int copySize)
{
	CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(buf);

	
	GameData userData;
	strcpy_s(userData.user_name, p->name);
	userData.user_exp = 1;
	userData.user_level = 1;
	userData.user_hp = 100;
	userData.user_x = rand()% LIMIT_X;
	userData.user_y = rand()% LIMIT_X;
	userData.user_visual = rand() % 4+1;

	_sessionMgr->objects[id]->init(&userData);

	
	_sessionMgr->LoginSession(id);

	AddUSerInDB(&userData);

	std::cout << std::endl;

}


void PacketHandler::ProcessChoiceCharactertPacket(int id, char* buf, int copySize)
{
	CS_CHOICECHAR_PACKET* p = reinterpret_cast<CS_CHOICECHAR_PACKET*>(buf);
	
	_sessionMgr->objects[id]->_visual = p->visual;
	_sessionMgr->LoginSession(id);

	AddVisualInfoInDB(_sessionMgr->objects[id]->_name, p->visual);
}

void PacketHandler::ProcessMovePacket(int id, char* buf, int copySize)
{
	CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(buf);
	_sessionMgr->objects[id]->last_move_time = packet->move_time;

	int x = _sessionMgr->objects[id]->_x;
	int y = _sessionMgr->objects[id]->_y;

	if (x >= LIMIT_X || x < 0 || y >= LIMIT_Y || y < 0) {
		std::cout << "잘못된 접근" << std::endl;
		return;
	}
	
	if (_mapMgr->IsCanGoCheck(packet->direction, x, y)) {
	
		_sessionMgr->objects[id]->_x = x;
		_sessionMgr->objects[id]->_y = y;
		_sessionMgr->MoveSession(id, packet);
	}

}

void PacketHandler::ProcessChattingPacket(int id, char* buf, int copySize)
{
	CS_CHAT_PACKET* packet = reinterpret_cast<CS_CHAT_PACKET*>(buf);


	_sessionMgr->BroadcastChatting(id, copySize - 3 ,&buf[3]);


}

void PacketHandler::ProcessAttackPacket(int id, char* buf, int copySize)
{
	CS_ATTACK_PACKET* packet = reinterpret_cast<CS_ATTACK_PACKET*>(buf);

	_sessionMgr->AttackSessionToNPC(id, packet->dir);

}

void PacketHandler::ProcessTeleportPacket(int id, char* buf, int copySize)
{
}

void PacketHandler::ProcessLogoutPacket(int id, char* buf, int copySize)
{
}

int PacketHandler::CheckUserInDB(const char* name,GameData* data)
{
	DBConnection* dbConn = _dbConnPool->Pop();
	dbConn->Unbind();

	SQLLEN outLen = 0;

	dbConn->BindCol(1, data->user_name, NAME_SIZE, &outLen);
	dbConn->BindCol(2, &data->user_level, &outLen);
	dbConn->BindCol(3, &data->user_exp, &outLen);
	dbConn->BindCol(4, &data->user_hp, &outLen);
	dbConn->BindCol(5, &data->user_x, &outLen);
	dbConn->BindCol(6, &data->user_y, &outLen);
	dbConn->BindCol(7, &data->user_visual, &outLen);

	SQLWCHAR query[1024];
	wsprintf(query, L"SELECT * FROM game_server.dbo.game_data_table WHERE user_name = '%s'", ConvertToWideChar(name));
	dbConn->Excute(query);

	if (dbConn->Fetch()==false) {
		_dbConnPool->Push(dbConn);
		return NOT_EXIST_IN_DB;
	}

	if (data->user_visual == 0) {
		_dbConnPool->Push(dbConn);
		return NOT_CHOICE_CHARACTER;
	}

	_dbConnPool->Push(dbConn);
	return NONE;
}

bool PacketHandler::AddUSerInDB(const char* name)
{
	DBConnection* dbConn = _dbConnPool->Pop();
	dbConn->Unbind();

	SQLLEN len = 0;
	dbConn->BindParam(1, name, &len);
	SQLWCHAR query[1024];

	wsprintf(query, 
		L"INSERT INTO game_server.dbo.game_data_table (user_name, user_level, user_exp, user_hp, user_x, user_y,user_visual)  \
		VALUES (\'%s\', %d, %d, %d, %d, %d, %d)", ConvertToWideChar(name),1,0,100, INIT_X_POS, INIT_Y_POS,0);

	dbConn->Excute(query);

	_dbConnPool->Push(dbConn);
	return true;


}
bool PacketHandler::AddUSerInDB(const GameData* gameData)
{
	//스트레스 테스트용
	DBConnection* dbConn = _dbConnPool->Pop();
	dbConn->Unbind();

	SQLLEN len = 0;
	dbConn->BindParam(1, gameData->user_name, &len);
	SQLWCHAR query[1024];

	wsprintf(query,
		L"INSERT INTO game_server.dbo.game_data_table (user_name, user_level, user_exp, user_hp, user_x, user_y,user_visual)  \
		VALUES (\'%s\', %d, %d, %d, %d, %d, %d)", ConvertToWideChar(gameData->user_name), 
		1, 0, 100, gameData->user_x, gameData->user_y, gameData->user_visual);

	dbConn->Excute(query);

	_dbConnPool->Push(dbConn);
	return true;


}

bool PacketHandler::AddVisualInfoInDB(const char* name, int visual)
{

	DBConnection* dbConn = _dbConnPool->Pop();
	dbConn->Unbind();

	SQLWCHAR query[1024];
	wsprintf(query,
		L"UPDATE game_server.dbo.game_data_table SET user_visual = %d		\
		WHERE user_name = '%s'",	visual, ConvertToWideChar(name));


	//wsprintf(query, L"UPDATE Table_3 SET user_x = %d, user_y = %d, user_EXP = %d, user_HP = %d, user_LEVEL = %d WHERE user_id = %d", x, y, exp, hp, level, keyid);
	SQLLEN len = 0;
	dbConn->BindParam(1, name, &len);
	dbConn->BindParam(7, &visual, &len);

	std::cout << "등록완료?" << std::endl;
	dbConn->Excute(query);

	_dbConnPool->Push(dbConn);
	return true;
}





