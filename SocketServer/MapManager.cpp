#include "stdafx.h"
#include "MapManager.h"


using namespace std;

MapManager::MapManager()
{
	///_moveInfo.reset();
}

void MapManager::InitMapInfo()
{
	//파일을 읽어와서 맵데이터를 저장한다.
	ifstream in{ "section1.txt",ios::binary};
	

	int temp;

	int i = 0;
	int j = 0;

	//못가는곳이 0, 갈수있는곳이 1
	while (in >> temp) {
		//TODO. 고쳐야한다@@@@
		_moveInfo[i][j] = 0;//!temp; 맵테스트 해야해서 일단 다 1로해놓음

		++j;
		if (j >= 150) {
			++i;
			j = 0;
		}
	}

}

//얘가 문제다. 
bool MapManager::IsCanGoCheck(int dir,int& x, int& y)
{

	switch (dir) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	}

	if (x >= LIMIT_X || x < 0 || y >= LIMIT_Y || y < 0) return false;

	//TODO!!!!-> 테스트용으로 다 0으로 세팅해놔서 일단 그렇다.고쳐야함
	return !_moveInfo[y][x];
}

bool MapManager::IsCanGoCheck(int x, int y)
{

	return _moveInfo[y][x];
}
