#include "stdafx.h"
#include "MapManager.h"


using namespace std;

MapManager::MapManager()
{
	///_moveInfo.reset();
}

void MapManager::InitMapInfo()
{
	//������ �о�ͼ� �ʵ����͸� �����Ѵ�.
	ifstream in{ "section1.txt",ios::binary};
	

	int temp;

	int i = 0;
	int j = 0;

	//�����°��� 0, �����ִ°��� 1
	while (in >> temp) {
		_moveInfo[i][j] = !temp;
		++j;
		if (j >= 150) {
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

	return _moveInfo[y][x];
}
