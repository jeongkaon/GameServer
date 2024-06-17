#pragma once
#include "Obejct.h"
#include "AStar.h"

class MapManager;

class NPC :public Object
{

	int _rangeX;
	int _rangeY;

	MapManager* _mapMgr;

public:
	std::atomic_bool	_is_active;		
	lua_State* _L;

	int _moveType;
	int _monType;	//peace or agro

	std::chrono::system_clock::time_point wakeupTime;

	AStar astar;
	std::vector<std::pair<int, int>> path;
	int _pathIndex;

public:
	static int TotalNpcCount;

public:
	NPC();
	NPC(int x, int y);

	void init(MapManager* mgr, int x, int y, int visual);
	

	void DoRandomMove();
	void DoAstarMove(int desx, int desy);

	void OnAttackSuccess(int visual);
	bool OnAttackReceived(int damage);

	void RecoverHP();
	//±Ê√£±‚

};



