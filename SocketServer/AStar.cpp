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

};

//�̵��Ҷ� ��� �����.
int cost[] =
{
	10, // UP
	10, // LEFT
	10, // DOWN
	10, // RIGHT


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

void AStar::init(int x, int y, int desx, int desy)
{
    startX = x;
    startY = y;
    destX = desx;
    destY = desy;

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

        int nodeXIdx = node.x - startX;
        int nodeYIdx = node.y - startY;

        if (closed[nodeYIdx][nodeXIdx])
            continue;
        if (best[nodeYIdx][nodeXIdx] < node.f)
            continue;

        closed[nodeYIdx][nodeXIdx] = true;

        if (node.x == destX && node.y == destY) {
            break;
        }

        // �밢�������Ϸ��� 8
        for (int dir = 0; dir < 4; dir++)
        {
            int nextX = node.x + front[dir].x;
            int nextY = node.y + front[dir].y;


            if (false == mapMgr->IsCanGoCheck(nextX, nextY)) {
                continue;
            }

            int nextXIdx = nextX - startX;
            int nextYIdx = nextY - startY;

            // �̹� �湮�� ���̸� ��ŵ
            if (closed[nextYIdx][nextXIdx]) {
                continue;
            }

            // ��� ���
            int g = node.g + cost[dir];
            int h = 10 * (abs(destY - nextY) + abs(destX - nextX));

            // �ٸ� ��ο��� �� ���� ���� ã������ ��ŵ
            if (best[nextYIdx][nextXIdx] <= g + h)
                continue;

            // ���� ����
            best[nextYIdx][nextXIdx] = g + h;
            pq.push(PQNode{ g + h, g, nextX, nextY });

            parent[{nextX, nextY}] = { node.x, node.y };
        }
    }

    // �Ųٷ� �Ž��� �ö󰣴�
    std::pair<int, int> pos{ destX, destY };

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