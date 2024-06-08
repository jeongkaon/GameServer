#pragma once
#include "protocol_2024.h"

constexpr int VIEW_RANGE = 10;
constexpr int SECTOR_SIZE = 10;
constexpr int SECTOR_NUM = W_WIDTH / SECTOR_SIZE;

enum CompleteType { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_MOVE, OP_PLAYER_MOVE };
enum SessionState { ST_FREE, ST_ALLOC, ST_INGAME };

enum MapInfo{ POSSIBLE, IMPOSSIBLE};

enum EventType { EV_RANDOM_MOVE };

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