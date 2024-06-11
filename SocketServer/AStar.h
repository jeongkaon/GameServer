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
	AStar();
	AStar(int x, int y, int searchRange);

	void init(int x, int y, int searchRange);
	
	void FindPath(MapManager* mapMgr, std::vector<std::pair<int, int>>* path);
};

