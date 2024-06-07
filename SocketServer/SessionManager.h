#pragma once
#include "Session.h"
#include "NPC.h"
#include "Sector.h"
#include "Obejct.h"


class SessionManager
{
public:
	std::array<Object*, MAX_USER + MAX_NPC> objects;

	//섹터가 npc에게도 필요하다......
	static Sector sector[SECTOR_NUM][SECTOR_NUM];


public:
	SessionManager();
	void Init();
	

	int AcceptClient(SOCKET& socket);
	int RetNewClientId();

	bool CanSee(int from, int to);

	
	int CheckLoginSession(int id);
	void LoginSession(int id, char* name);
	void LoginSession(int id, int visual);

	void MoveSession(int id, CS_MOVE_PACKET* packet);
	void CharChoiceSession(int id);

	void disconnect(int key);


};

