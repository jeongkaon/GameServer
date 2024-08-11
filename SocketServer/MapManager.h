#pragma once
#include <bitset>
class MapManager
{
	
	std::array<std::bitset<LIMIT_X>, LIMIT_Y> _moveInfo;


public:
	MapManager();
	void InitMapInfo();

	bool IsCanGoCheck(int dir, int& x, int& y);
	bool IsCanGoCheck(int x, int y);


};

