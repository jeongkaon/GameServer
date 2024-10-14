#include "stdafx.h"
#include "AStar.h"
#include "MapManager.h"


struct Pos { int x; int y; };

const int AstaSize = 20;
const int HalfAstaSize = 10;

Pos front[] =
{
	Pos { -1, 0},	// UP
	Pos { 0, -1},	// LEFT
	Pos { 1, 0},	// DOWN
	Pos { 0, 1},	// RIGHT

};

//이동할때 드는 비용임.
int cost[] =
{
	10, // UP
	10, // LEFT
	10, // DOWN
	10, // RIGHT


};

AStar::AStar()
{

}

AStar::AStar(int startX, int startY, int destX, int destY)
    : startX(startX), startY(startY), destX(destX), destY(destY)
{

}
void AStar::init(int x, int y, int destx, int desty)
{
    startX = x;
    startY = y;


    destX = destx;
    destY = desty;



}
void AStar::FindPath(MapManager* mapMgr, std::vector<std::pair<int, int>>* path)
{

    std::vector<std::vector<bool>> closed(AstaSize, std::vector<bool>(AstaSize, false));
    std::vector<std::vector<int>> best(AstaSize, std::vector<int>(AstaSize, INT32_MAX));
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;


    std::map<std::pair<int, int>, std::pair<int, int>> parent;

    int g = 0;
    int h = 10 * (abs(destY - startY) + abs(destX - startX));


    pq.push(PQNode{ g + h, g, startX,startY });
    best[HalfAstaSize][HalfAstaSize] = g + h;

    parent[{startX, startY}] = { startX, startY }; 

    while (!pq.empty()) {
        PQNode node = pq.top();
        pq.pop();

        int nodeXIdx = node.x - startX + HalfAstaSize;
        int nodeYIdx = node.y - startY + HalfAstaSize;

        if (nodeXIdx < 0 || nodeXIdx >= AstaSize || nodeYIdx < 0 || nodeYIdx >= AstaSize)
            continue;

        //이미 방문한 곳이면 스킵
        if (closed[nodeYIdx][nodeXIdx]) continue;

        closed[nodeYIdx][nodeXIdx] = true;

        //목적지에 도착하면 종료
        if (node.x == destX && node.y == destY) {
            break;
        }

        for (int dir = 0; dir < 4; dir++) {
            int nextX = node.x + front[dir].x;
            int nextY = node.y + front[dir].y;

            int nextXIdx = nextX - startX + HalfAstaSize;
            int nextYIdx = nextY - startY + HalfAstaSize;

            //인덱스가 유효한 범위 내에 있는지 확인
            if (nextXIdx < 0 || nextXIdx >= AstaSize || nextYIdx < 0 || nextYIdx >= AstaSize)
                continue;

            //갈 수 있는 지역인지 확인
            if (!mapMgr->IsCanGoCheck(nextX, nextY)) {
                continue;
            }

            //이미 방문한 곳이면 스킵
            if (closed[nextYIdx][nextXIdx]) {
                continue;
            }

            //비용 계산
            int g = node.g + cost[dir];
            int h = 10 * (abs(destY - nextY) + abs(destX - nextX));

            //다른 경로에서 더 빠른 길을 찾았으면 스킵
            if (best[nextYIdx][nextXIdx] <= g + h)
                continue;

            //예약 진행
            best[nextYIdx][nextXIdx] = g + h;
            pq.push(PQNode{ g + h, g, nextX, nextY });

            parent[{nextX, nextY}] = { node.x, node.y };
        }
    }

    //거꾸로 거슬러 올라간다
    std::pair<int, int> pos{ destX, destY };
    path->clear();
    while (true) {
        path->push_back(pos);

        //시작점은 자신이 곧 부모이다
        if (pos == parent[pos]) {
            break;
        }

        pos = parent[pos];
    }

    std::reverse(path->begin(), path->end());

  
}
