#pragma once
#include "Obejct.h"

class NPC :public Object
{
public:
	int _type;
	int _moveType;

	std::atomic_bool	_is_active;		// ������ �÷��̾ �ִ°�?

	float _speed;
	//TODO. �״½ð� �־���Ѵ�.
	//TODO. ����߰��־��Ѵ�.

	static int TotalNpcCount;


public:
	NPC();
	NPC(int x, int y);

	void init();
	void DoRandomMove();
	void BroadCastingInSection(void* buf);

};



