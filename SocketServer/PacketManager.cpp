#include "stdafx.h"
#include "PacketManager.h"
#include "SessionManager.h"
#include "MapManager.h"
#include "DBConnectionPool.h"

wchar_t* ConvertToWideChar(const char* str) {
	// �ʿ��� ���� ũ�� ���
	size_t len = strlen(str) + 1;
	wchar_t* buffer = new wchar_t[len];

	// ��ȯ
	mbstowcs(buffer, str, len);

	return buffer;
}

void PacketManager::Init(SessionManager* sessionMgr, MapManager* mapMgr, DBConnectionPool* dbConnPool)
{
	_sessionMgr = sessionMgr;
	_mapMgr = mapMgr;
	_dbConnPool = dbConnPool;

	//ȸ�������� ����� ��Ʈ���� �׽�Ʈ�� �Ǵ��� �˾ƾ���
	_recvFuntionMap[(int)CS_REGIST] = &PacketManager::ProcessRegistPacket;

	_recvFuntionMap[(int)CS_LOGIN] = &PacketManager::ProcessLoginPacket;
	_recvFuntionMap[(int)CS_CHOICE_CHARACTER] = &PacketManager::ProcessChoiceCharactertPacket;
	_recvFuntionMap[(int)CS_MOVE] = &PacketManager::ProcessMovePacket;
	_recvFuntionMap[(int)CS_CHAT] = &PacketManager::ProcessChattingPacket;
	_recvFuntionMap[(int)CS_ATTACK] = &PacketManager::ProcessAttackPacket;
	_recvFuntionMap[(int)CS_TELEPORT] = &PacketManager::ProcessTeleportPacket;
	_recvFuntionMap[(int)CS_LOGOUT] = &PacketManager::ProcessLogoutPacket;
}

void PacketManager::ProcessRecvPacket(int id, char* buf, int copySize)
{
	//��� ������ �̾Ƽ� ����������.
	PACKET_HEADER* pHeader = reinterpret_cast<PACKET_HEADER*>(buf);

	auto iter = _recvFuntionMap.find(pHeader->type);

	if (iter != _recvFuntionMap.end())
	{
		(this->*(iter->second))(id, buf, copySize);
	}
	else {
		return;
	}

}


void PacketManager::ProcessRegistPacket(int id, char* buf, int copySize)
{
	CS_REGIST_PACKET* p = reinterpret_cast<CS_REGIST_PACKET*>(buf);
	//db�� ����ؾ��ҰŰ�����??

}

void PacketManager::ProcessLoginPacket(int id, char* buf, int copySize)
{
	CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(buf);
	
	GameData userData{};
	
	int res = CheckUserInDB(p->name, userData);

	//���� userData�� ������� session������ ������Ʈ
	//TODO. �α���� �� �����غ�����

	switch (res)
	{
	case NONE:
		//TODO.db���� ���� �ڷ� ���� �α��ϴ°ŷ� ����
		_sessionMgr->LoginSession(id, userData.user_name);
		break;
	case NOT_EXIST_IN_DB:
		//DB�� �����ϱ� ���Ӱ� ����
		AddUSerInDB(p->name);

		break;
	case NOT_CHOICE_CHARACTER:
		_sessionMgr->CharChoiceSession(id);


		break;
	default:
		break;
	}



}

void PacketManager::ProcessChoiceCharactertPacket(int id, char* buf, int copySize)
{
	CS_CHOICECHAR_PACKET* p = reinterpret_cast<CS_CHOICECHAR_PACKET*>(buf);
	
	//ĳ���� ���� �����ؾ��Ѵ�. �׸��� �α� ��Ŷ���� �ؾ���
	//�̸��� ��ܾ��ϴµ�...���
	//->�ش� id�� �̹� d
	_sessionMgr->LoginSession(id, p->visual);



}

void PacketManager::ProcessMovePacket(int id, char* buf, int copySize)
{
	CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(buf);
	_sessionMgr->objects[id]->last_move_time = packet->move_time;

	int x = _sessionMgr->objects[id]->_x;
	int y = _sessionMgr->objects[id]->_y;
			
	if (_mapMgr->IsCanGoCheck(packet->direction, x, y)) {
	
		_sessionMgr->objects[id]->_x = x;
		_sessionMgr->objects[id]->_y = y;
		_sessionMgr->MoveSession(id, packet);
	}

}

void PacketManager::ProcessChattingPacket(int id, char* buf, int copySize)
{


}

void PacketManager::ProcessAttackPacket(int id, char* buf, int copySize)
{
	CS_ATTACK_PACKET* packet = reinterpret_cast<CS_ATTACK_PACKET*>(buf);

	_sessionMgr->AttackSession(id, packet->dir);

}

void PacketManager::ProcessTeleportPacket(int id, char* buf, int copySize)
{
}

void PacketManager::ProcessLogoutPacket(int id, char* buf, int copySize)
{
}

int PacketManager::CheckUserInDB(const char* name,GameData& data)
{
	//name ���� �ִ��� Ȯ���ϴ� �Լ�
	DBConnection* dbConn = _dbConnPool->Pop();
	dbConn->Unbind();

	SQLLEN outLen = 0;

	dbConn->BindCol(1, data.user_name, NAME_SIZE, &outLen);
	dbConn->BindCol(2, &data.user_level, &outLen);
	dbConn->BindCol(3, &data.user_exp, &outLen);
	dbConn->BindCol(4, &data.user_hp, &outLen);
	dbConn->BindCol(5, &data.user_x, &outLen);
	dbConn->BindCol(6, &data.user_y, &outLen);
	dbConn->BindCol(7, &data.user_visual, &outLen);

	SQLWCHAR query[1024];
	wsprintf(query, L"SELECT * FROM game_server.dbo.game_data_table WHERE user_name = '%s'", ConvertToWideChar(name));
	dbConn->Excute(query);


	if (dbConn->Fetch()==false) {
		_dbConnPool->Push(dbConn);
		return NOT_EXIST_IN_DB;
	}


	if (data.user_visual == 0) {
		_dbConnPool->Push(dbConn);
		return NOT_CHOICE_CHARACTER;
	}

	_dbConnPool->Push(dbConn);
	return NONE;
}

bool PacketManager::AddUSerInDB(const char* name)
{
	DBConnection* dbConn = _dbConnPool->Pop();
	dbConn->Unbind();

	SQLLEN len = 0;
	dbConn->BindParam(1, name, &len);
	SQLWCHAR query[1024];

	wsprintf(query, 
		L"INSERT INTO game_server.dbo.game_data_table (user_name, user_level, user_exp, user_hp, user_x, user_y,user_visual)  \
		VALUES (\'%s\', %d, %d, %d, %d, %d, %d)", ConvertToWideChar(name),0,0,100,0,0,0);

	

	dbConn->Excute(query);

	_dbConnPool->Push(dbConn);
	return true;


}




