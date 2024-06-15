#include "stdafx.h"
#include "AStar.h"
#include "MapManager.h"


struct Pos { int x; int y; };

Pos front[] =
{
	Pos { -1, 0},	// UP
	Pos { 0, -1},	// LEFT
	Pos { 1, 0},	// DOWN
	Pos { 0, 1},	// RIGHT

	//�밢���� ���� �ǽ����� �߰��� �ֵ�
	Pos {-1, -1},	// UP_LEFT
	Pos {1, -1},	// DOWN_LEFT
	Pos {1, 1},		// DOWN_RIGHT
	Pos {-1, 1},	// UP_RIGHT
};

//�̵��Ҷ� ��� �����.
int cost[] =
{
	10, // UP
	10, // LEFT
	10, // DOWN
	10, // RIGHT

	//�밢���� 14������ ����
	14,
	14,
	14,
	14
};


AStar::AStar()
	:closed(40, std::vector<bool>(40, false)),
	best(40, std::vector<int>(40, INT32_MAX))

{

}

AStar::AStar(int x, int y, int searchRange)
	: startX(x), startY(y),destX(x+ searchRange),destY(y+ searchRange),
	closed(searchRange, std::vector<bool>(searchRange, false)),
	best(searchRange, std::vector<int>(searchRange, INT32_MAX))
{
	int g = 0;
	int h = 10 * (abs(destY - startY) + abs(destX - startX));
	pq.push(PQNode{ g + h, g, startX,startY });
	best[0][0] = g + h;
	//best[0][0] = g + h;
	parent[{x, y}] = { x,y };

}

void AStar::init(int x, int y, int searchRange)
{
	startX = x;
	startY = y;
	destX = x + searchRange;
	destY = y + searchRange;

	int g = 0;
	int h = 10 * (abs(destY - startY) + abs(destX - startX));
	pq.push(PQNode{ g + h, g, startX,startY });

	best[0][0] = g + h;
	parent[{x, y}] = { x,y };

}

void AStar::FindPath(MapManager* mapMgr, std::vector<std::pair<int, int>>* path)
{
	while (false == pq.empty())
	{
		PQNode node = pq.top();
		pq.pop();

		if (closed[node.y - startY][node.x - startX])
			continue;
		if (best[node.y - startY][node.x - startX] < node.f)
			continue;

		closed[node.y - startY][node.x - startX] = true;

		if (node.x == destX && node.y == destY) {
			break;
		}

		//�밢�������Ϸ��� 8
		for (int dir = 0; dir < 4; dir++)
		{
			int nextX = node.x + front[dir].x;
			int nextY = node.y + front[dir].y;
			
			// �� �� �ִ� ������ �´��� Ȯ��
			if (nextX > startX + destX || nextX <startX ||
				nextY >startY + destY || nextY < startY) continue;

			int moveX = nextX;
			int moveY = nextY;
		
			if (false == mapMgr->IsCanGoCheck(dir, moveX, moveY)) {
				continue;
			}
			nextX = moveX;
			nextY = moveY;

			// �̹� �湮�� ���̸� ��ŵ
			if (closed[nextY-startY][nextX - startX]) {
				continue;
			}

			// ��� ���
			int g = node.g + cost[dir];
			int h = 10 * (abs(destY - nextY) + abs(destX - nextX));
			// �ٸ� ��ο��� �� ���� ���� ã������ ��ŵ
			if (best[nextY - startY][nextX - startX] <= g + h)
				continue;

			// ���� ����
			best[nextY - startY][nextX - startX] = g + h;
			pq.push(PQNode{ g + h, g, nextX,nextY });

			parent[{nextX,nextY}] = {node.x,node.y};

		}
	}


	// �Ųٷ� �Ž��� �ö󰣴�
	std::pair<int,int> pos{ destX,destY };

	while (true)
	{
		path->push_back(pos);

		// �������� �ڽ��� �� �θ��̴�
		if (pos == parent[pos]) {
			break;
		}
	

		pos = parent[pos];
	}

	std::reverse(path->begin(), path->end());

}
