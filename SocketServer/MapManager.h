#pragma once
#include <bitset>
class MapManager
{
	
	//bit를 사용하는거로 바꾸자 2000 2000 이니까 
	//일단 bool로 하고 bit로 바꾸는방법 강구하기
	bool _moveInfo[150][150]{};

public:
	MapManager();
	void Init();
	void InitNpc();

	bool IsCanGoCheck(int dir, int& x, int& y);

	//맵매니져에서 npc위치랑, 충돌이런거 다 체크하는거로 하자.ㄴ

	

};

