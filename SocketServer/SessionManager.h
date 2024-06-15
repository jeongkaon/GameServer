#pragma once
#include "Session.h"
#include "NPC.h"
#include "Sector.h"
#include "Obejct.h"
class Server;



class SessionManager
{
public:

	//npc위치 로딩해야한다.
	//TODO. 지금은 크기 150인데 이만으로 변경해야한다.

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
	

	int CheckLoginSession(int id);		//TODO. 기존에접속중인지 확인해야함
	//GameData인자로 받아서 그거로 로긴하는거임
	void LoginSession(int id);

	void MoveSession(int id, CS_MOVE_PACKET* packet);
	void CharChoiceSession(int id);

	void disconnect(int key);

	//npc관련
	void NpcRandomMove(int id);
	void NpcAstarMove(int id);

	//공격관련
	void AttackSessionToNPC(int id, char dir);
	void Attack(int npcId, int id);		//중복코드 따로 함수로 구현

	void RespawnNPC(int npcId);

};
