#pragma once
#include "protocol_2024.h"

constexpr int VIEW_RANGE = 5;
constexpr int SECTOR_SIZE = 10;
constexpr int SECTOR_NUM = W_WIDTH / SECTOR_SIZE;

enum CompleteType { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_MOVE, OP_PLAYER_MOVE };
enum SessionState { ST_FREE, ST_ALLOC, ST_INGAME };

enum MapInfo{ POSSIBLE, IMPOSSIBLE};

struct GameData {
	char user_name[NAME_SIZE];
	int user_level;
	int user_exp;
	int user_hp;
	int user_x;
	int user_y;
	int user_visual;
	
};

