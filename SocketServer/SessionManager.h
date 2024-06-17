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
	

	int CheckLoginSession(int id);		//TODO. ���������������� Ȯ���ؾ���
	//GameData���ڷ� �޾Ƽ� �װŷ� �α��ϴ°���
	void LoginSession(int id);

	void MoveSession(int id, CS_MOVE_PACKET* packet);
	void CharChoiceSession(int id);

	void disconnect(int key);

	//npc����
	void NpcRandomMove(int id);
	void NpcAstarMove(int id);
	bool NpcAgroActive(int npc, int plyaer);
	void SleepNPC(int id);

	//���ݰ���
	void AttackSessionToNPC(int id, char dir);
	void Attack(int npcId, int id);		//�ߺ��ڵ� ���� �Լ��� ����

	void RespawnNPC(int npcId);

	//ä��
	void BroadcastChatting(int id, int len, void* chat);

};
