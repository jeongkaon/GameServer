#pragma once
#include "Obejct.h"

class NPC :public Object
{
	bool isRoaming;
public:
	std::atomic_bool	_is_active;		
	
	//TODO. 죽는시간 넣어야한다.

	lua_State* _L;


	static int TotalNpcCount;


public:
	NPC();
	NPC(int x, int y);

	void init();
	void init(int x, int y, int visual);

	void DoRandomMove();

};



