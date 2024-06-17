#pragma once
#include "Session.h"
#include "NPC.h"
#include "Sector.h"
#include "Obejct.h"
class Server;



class SessionManager
{
public:


	std::array<std::array<char, LIMIT_X>, LIMIT_Y> _npcInfo;

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
	bool NpcAgroActive(int npc, int plyaer);
	void SleepNPC(int id);

	//공격관련
	void AttackSessionToNPC(int id, char dir);
	void Attack(int npcId, int id);		//중복코드 따로 함수로 구현

	void RespawnNPC(int npcId);

	//채팅
	void BroadcastChatting(int id, int len, void* chat);

};
