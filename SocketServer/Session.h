#pragma once
#include "ExpOver.h"
#include "Obejct.h"

class Session : public Object
{
	ExpOver _recvOver;
	public:
	SOCKET		_socket;

	int		_prevRemain;		//�̰� ��Ŷ�������Ҷ� ����Ѵ�.
	
	int _exp;
	int _level;

public:
	Session();

	void DoRecv();
	void DoSend(void* packet);

	void SendLoginPacket();
	void SendMovePacket(char dir);
	void SendMovePacket(int id, int x, int y, int time,char dir);
	void SendAddPlayerPacket(int id, char* name, int x, int y,int visual);
	void SendRemovePlayerPacket(int id);
	void SendChatPacket(int id, const char* mess);
	void SendChoiceCharPacket();

	void OnAttackSuccess(int type);
	void OnAttackReceived(int damage);

};
