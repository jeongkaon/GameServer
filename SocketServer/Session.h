#pragma once
#include "ExpOver.h"
#include "Obejct.h"
#include "MemoryPool.h"
class Session : public Object
{
	ExpOver _recvOver;
	public:
	SOCKET		_socket;

	int		_prevRemain;		
	
	int _exp;
	int _level;

	static MemoryPool* _memPool;

public:

	static void SetMemoryPool(MemoryPool* ptr);

	Session();

	void init(void* p);

	void DoRecv();
	void DoSend(void* packet);

	void SendLoginPacket();
	void SendMovePacket(char dir);
	void SendMovePacket(int id, int x, int y, int time,char dir);
	void SendAddPlayerPacket(int id, char* name, int x, int y,int visual);
	void SendRemovePlayerPacket(int id);
	void SendChatPacket(int id, const char* mess);
	void SendChoiceCharPacket();
	void SendAttackSuccessPakcet(int npcId, int damage);
	

	void SendPlayerGetExpAndLv(int npcId);
	void SendReceivedDamage(int npcId, int damage);

	bool OnAttackReceived(int damage);
	void UpdatePlayerExpAndLevel(int visual, int npcId);
	
};

