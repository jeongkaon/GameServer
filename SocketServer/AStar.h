#pragma once
class MapManager;

struct PQNode {
	bool operator<(const PQNode& other) const { return f < other.f; }
	bool operator>(const PQNode& other) const { return f > other.f; }

	int	f; // f = g + h
	int	g;

	int x;
	int y;

};

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

class AStar
{
	int startX;
	int startY;

	int destX;
	int destY;


	std::vector<std::vector<bool>> closed;

	std::vector<std::vector<int>> best; 

	std::map<std::pair<int, int>, std::pair<int, int>> parent;
	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	


public:
	AStar(){}
	AStar(int x, int y, int range);
	
	void FindPath(MapManager* mapMgr, std::vector<std::pair<int, int>>* path);
};

