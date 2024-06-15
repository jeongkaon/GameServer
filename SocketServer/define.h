#pragma once
#include "protocol_2024.h"

constexpr int SECTOR_SIZE = 10;
constexpr int SECTOR_NUM = W_WIDTH / SECTOR_SIZE;
constexpr int ATTACK_RANGE = 1;
constexpr int INIT_X_POS = 3;
constexpr int INIT_Y_POS = 1;

enum NpcType{MOVE_FIXED, MOVE_ROAMING, TYPE_AGRO, TYPE_PEACE};

enum CompleteType { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_MOVE, 
	OP_PLAYER_MOVE, OP_NPC_MOVE_ACTIVE,OP_RECOVER_HP, OP_NPC_RESPAWN,
};
enum SessionState { ST_FREE, ST_ALLOC, ST_INGAME };

enum MapInfo{ POSSIBLE, IMPOSSIBLE};

enum EventType { EV_RANDOM_MOVE ,EV_ACTIVE_MOVE, EV_RECOVER_HP, EV_NPC_DIE};

//레벨요구 경험치
enum LevelRequired {
	LEVEL2 = 1,
	LEVEL3 = 2,
	LEVEL4 = 4,
	LEVEL5 = 8
};
enum MaxHp {
	MAX_HP1 = 100,
	MAX_HP2 = 200,
	MAX_HP3 = 300,
	MAX_HP4 = 400,
	MAX_HP5 = 500
};

struct GameData {
	char user_name[NAME_SIZE];
	int user_level;
	int user_exp;
	int user_hp;
	int user_x;
	int user_y;
	int user_visual;
	
};

struct TimerEvent {		//타이머
	int obj_id;
	std::chrono::system_clock::time_point wakeup_time;
	EventType event_id;		//event종류->무엇을에 해당한다->EventType
	int target_id;


	constexpr bool operator < (const TimerEvent& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};