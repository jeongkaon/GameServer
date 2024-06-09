#pragma once
#include "Obejct.h"

class NPC :public Object
{


public:
	std::atomic_bool	_is_active;		
	lua_State* _L;
	
	//TODO. 죽는시간 넣어야한다.
	std::chrono::system_clock::time_point dieTime;
	
	bool _isMove;


public:
	static int TotalNpcCount;

public:
	NPC();
	NPC(int x, int y);

	void init();
	void init(int x, int y, int visual);

	void DoRandomMove();

	void OnAttackSuccess(int visual);
	void OnAttackReceived(int damage);

};



