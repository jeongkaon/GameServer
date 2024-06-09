#pragma once
#include "Session.h"
#include "NPC.h"
#include "Sector.h"
#include "Obejct.h"
class Server;



class SessionManager
{
public:

	//npc��ġ �ε��ؾ��Ѵ�.
	//TODO. ������ ũ�� 150�ε� �̸����� �����ؾ��Ѵ�.

	std::array<std::array<int,150>,150> _npcInfo;

	std::array<Object*, MAX_USER + MAX_NPC> objects;
	static Sector sector[SECTOR_NUM][SECTOR_NUM];

	Server* server;


public:
	SessionManager();
	void Init();

	int AcceptClient(SOCKET& socket);
	int RetNewClientId();

	bool CanSee(int from, int to);

	
	void SetUserGameData(int id, GameData& data);

	int CheckLoginSession(int id);		//TODO. ���������������� Ȯ���ؾ���
	void LoginSession(int id, char* name);
	void LoginSession(int id, int visual);

	void MoveSession(int id, CS_MOVE_PACKET* packet);
	void CharChoiceSession(int id);

	void disconnect(int key);

	//npc����
	void NpcRandomMove(int id);

};
