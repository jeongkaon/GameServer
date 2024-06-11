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
	
	//TODO. �״½ð� �־���Ѵ�.->�ʿ���� �� 30�� �ٷ� �־��ָ��
	
	bool _isMove;
	std::vector<std::pair<int, int>> path;
	AStar astar;



public:
	static int TotalNpcCount;

public:
	NPC();
	NPC(int x, int y);

	void init(int x, int y, int visual);

	void DoRandomMove();

	void OnAttackSuccess(int visual);
	bool OnAttackReceived(int damage);

	//��ã��

};



