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

	int startX, startY, destX, destY;


public:
	AStar();
	AStar(int startX, int startY, int destX, int destY);

	void init(int startX, int startY, int destX, int destY);


	void FindPath(MapManager* mapMgr, std::vector<std::pair<int, int>>* path);

};
