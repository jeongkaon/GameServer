#pragma once
#include "Obejct.h"

class NPC :public Object
{
public:
	int _type;
	int _moveType;

	std::atomic_bool	_is_active;		// 주위에 플레이어가 있는가?

	float _speed;
	//TODO. 죽는시간 넣어야한다.
	//TODO. 루아추가애야한다.

	static int TotalNpcCount;


public:
	NPC();
	NPC(int x, int y);

	void init();
	void DoRandomMove();
	void BroadCastingInSection(void* buf);

};



