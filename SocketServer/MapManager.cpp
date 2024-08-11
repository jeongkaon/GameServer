#include "stdafx.h"
#include "MapManager.h"


using namespace std;

MapManager::MapManager()
{
}

void MapManager::InitMapInfo()
{
	ifstream in{ "collision.txt",ios::binary};
	int temp;

	int i = 0;
	int j = 0;

	while (in >> temp) {
		_moveInfo[i][j] = !temp;

		++j;
		if (j >= LIMIT_X) {
			++i;
			j = 0;
		}
	}
	
}

bool MapManager::IsCanGoCheck(int dir,int& x, int& y)
{

	switch (dir) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	}

	if (x >= LIMIT_X || x < 0 || y >= LIMIT_Y || y < 0) return false;

	return _moveInfo[y][x];
}

bool MapManager::IsCanGoCheck(int x, int y)
{
	if (x >= LIMIT_X || x < 0 || y >= LIMIT_Y || y < 0) return false;


	return _moveInfo[y][x];
}
