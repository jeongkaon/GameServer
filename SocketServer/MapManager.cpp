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
		//TODO. ���ľ��Ѵ�@@@@
		_moveInfo[i][j] = 0;//!temp; ���׽�Ʈ �ؾ��ؼ� �ϴ� �� 1���س���

		++j;
		if (j >= 150) {
			++i;
			j = 0;
		}
	}

}

//�갡 ������. 
bool MapManager::IsCanGoCheck(int dir,int& x, int& y)
{

	switch (dir) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	}

	if (x >= LIMIT_X || x < 0 || y >= LIMIT_Y || y < 0) return false;

	//TODO!!!!-> �׽�Ʈ������ �� 0���� �����س��� �ϴ� �׷���.���ľ���
	return !_moveInfo[y][x];
}

bool MapManager::IsCanGoCheck(int x, int y)
{

	return _moveInfo[y][x];
}
