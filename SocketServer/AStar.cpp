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

	//대각선을 위한 실습으로 추가한 애들
	Pos {-1, -1},	// UP_LEFT
	Pos {1, -1},	// DOWN_LEFT
	Pos {1, 1},		// DOWN_RIGHT
	Pos {-1, 1},	// UP_RIGHT
};

//이동할때 드는 비용임.
int cost[] =
{
	10, // UP
	10, // LEFT
	10, // DOWN
	10, // RIGHT

	//대각선은 14정도로 설정
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

		//대각선까지하려면 8
		for (int dir = 0; dir < 4; dir++)
		{
			int nextX = node.x + front[dir].x;
			int nextY = node.y + front[dir].y;
			
			// 갈 수 있는 지역은 맞는지 확인
			if (nextX > startX + destX || nextX <startX ||
				nextY >startY + destY || nextY < startY) continue;

			int moveX = nextX;
			int moveY = nextY;
		
			if (false == mapMgr->IsCanGoCheck(dir, moveX, moveY)) {
				continue;
			}
			nextX = moveX;
			nextY = moveY;

			// 이미 방문한 곳이면 스킵
			if (closed[nextY-startY][nextX - startX]) {
				continue;
			}

			// 비용 계산
			int g = node.g + cost[dir];
			int h = 10 * (abs(destY - nextY) + abs(destX - nextX));
			// 다른 경로에서 더 빠른 길을 찾았으면 스킵
			if (best[nextY - startY][nextX - startX] <= g + h)
				continue;

			// 예약 진행
			best[nextY - startY][nextX - startX] = g + h;
			pq.push(PQNode{ g + h, g, nextX,nextY });

			parent[{nextX,nextY}] = {node.x,node.y};

		}
	}


	// 거꾸로 거슬러 올라간다
	std::pair<int,int> pos{ destX,destY };

	while (true)
	{
		path->push_back(pos);

		// 시작점은 자신이 곧 부모이다
		if (pos == parent[pos]) {
			break;
		}
	

		pos = parent[pos];
	}

	std::reverse(path->begin(), path->end());

}
