#pragma once
#include "Obejct.h"



#include "AStar.h"

class NPC :public Object
{

	int _rangeX;
	int _rangeY;

public:
	std::atomic_bool	_is_active;		
	lua_State* _L;
	
	
	//bool _isMove;
	std::vector<std::pair<int, int>> path;
	//AStar astar;

	int _moveType;
	int _monType;	//peace or agro

	std::chrono::system_clock::time_point wakeupTime;



public:
	static int TotalNpcCount;

public:
	NPC();
	NPC(int x, int y);

	void init(int x, int y, int visual);

	void DoRandomMove();

	void OnAttackSuccess(int visual);
	bool OnAttackReceived(int damage);

	void RecoverHP();
	//±Ê√£±‚

};



