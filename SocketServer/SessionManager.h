#pragma once
#include "Session.h"
#include "NPC.h"
#include "Sector.h"
#include "Obejct.h"
class Server;
class MemoryPool;


class SessionManager
{
public:
	std::array<std::array<char, LIMIT_X>, LIMIT_Y> _npcInfo;
	std::array<Object*, MAX_USER + MAX_NPC> objects;

	static Sector sector[SECTOR_NUM][SECTOR_NUM];

	Server* server;

public:
	SessionManager();
	void Init(MemoryPool* ptr);

	int AcceptClient(SOCKET& socket);
	int RetNewClientId();

	bool CanSee(int from, int to);
	

	int CheckLoginSession(int id);		
	void LoginSession(int id);

	void MoveSession(int id, CS_MOVE_PACKET* packet);
	void CharChoiceSession(int id);

	unordered_set<int> UpdateViewlistInSection(const int curCol, const int curRow, const int id);

	void disconnect(int key);

	//npc관련
	void NpcRandomMove(int id);
	bool NpcAstarMove(int id, int target);
	void NpcAttackedMove(int id);

	bool NpcAgroActive(int npc, int plyaer);
	void SleepNPC(int id);

	//공격관련
	void AttackSessionToNPC(int id, char dir);
	void Attack(int npcId, int id);		

	void RespawnNPC(int npcId);

	//채팅
	void BroadcastChatting(int id, int len, void* chat);

};
