#include "stdafx.h"
#include "AStar.h"
#include "MapManager.h"

AStar::AStar(int x, int y, int range)
	: startX(x), startY(y),destX(x+range),destY(y+range),
	closed(range, std::vector<bool>(range, false)),
	best(range, std::vector<int>(range, INT32_MAX))
{
	int g = 0;
	int h = 10 * (abs(destY - startY) + abs(destX - startX));
	pq.push(PQNode{ g + h, g, startX,startY });
	best[startY][startX] = g + h;
	parent[{x, y}] = { x,y };

}

void AStar::FindPath(MapManager* mapMgr, std::vector<std::pair<int, int>>* path)
{
	while (false == pq.empty())
	{
		PQNode node = pq.top();
		pq.pop();

		if (closed[node.y][node.x])
			continue;
		if (best[node.y][node.x] < node.f)
			continue;

		closed[node.y][node.x] = true;

		if (node.x == destX && node.y == destY) {
			break;
		}

		//대각선까지하려면 8
		for (int dir = 0; dir < 4; dir++)
		{
			int nextX = node.x + front[dir].x;
			int nextY = node.y + front[dir].y;

			// 갈 수 있는 지역은 맞는지 확인
			if (false == mapMgr->IsCanGoCheck(dir, nextX, nextY)) {
				continue;
			}

			// 이미 방문한 곳이면 스킵
			if (closed[nextY][nextX]) {
				continue;
			}

			// 비용 계산
			int g = node.g + cost[dir];
			int h = 10 * (abs(destY - nextY) + abs(destX - nextX));
			// 다른 경로에서 더 빠른 길을 찾았으면 스킵
			if (best[nextY][nextX] <= g + h)
				continue;

			// 예약 진행
			best[nextY][nextX] = g + h;
			pq.push(PQNode{ g + h, g, nextX,nextY });

			parent[{nextX,nextY}] = { nextX,nextY};

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
