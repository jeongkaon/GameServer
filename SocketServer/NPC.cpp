#include "stdafx.h"
#include "NPC.h"
#include "SessionManager.h"
#include "Sector.h"
#include "LuaFunction.h"


int NPC::TotalNpcCount = 0;
NPC::NPC()
{

	sprintf_s(_name, "NPC%d", _id);

}
NPC::NPC(int x, int y)
{

	//루아

}


void NPC::init()
{
	//TODO.초기위치도 바꿔야함. -> 생성자에 있는거 옮겨오자!
	_state = ST_INGAME;
	SessionManager::sector[_sectorCol][_sectorRow].InsertObjectInSector(_id);

	sprintf_s(_name, "NPC%d", _id);

	_L = luaL_newstate();


	luaL_openlibs(_L);
	luaL_loadfile(_L, "npc.lua");
	lua_pcall(_L, 0, 0, 0);

	lua_getglobal(_L, "set_uid");
	lua_pushnumber(_L, _id);
	lua_pcall(_L, 1, 0, 0);

	lua_register(_L, "API_SendMessage", API_SendMessage);
	lua_register(_L, "API_get_x", API_get_x);
	lua_register(_L, "API_get_y", API_get_y);


}

void NPC::init(int x, int y, int visual)
{

	_id = MAX_USER + TotalNpcCount++;



	_state = ST_INGAME;

	_x = x;
	_y = y;
	_visual = visual;

	//id가아니라 type을 출력하는거로 일단 하겠다.
	sprintf_s(_name, "NPC%d", _id);


	switch (visual)
	{
	case PEACE_FIXED:
		break;
	case PEACE_ROAMING:
		break;
	case AGRO_FIXED:
		break;
	case AGRO_ROAMING:
		break;

	default:
		break;
	}

	_is_active = false;
	

	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;


	//흠..왜그럴까..
	SessionManager::sector[_sectorCol][_sectorRow].InsertObjectInSector(_id);
	_L = luaL_newstate();


	luaL_openlibs(_L);
	luaL_loadfile(_L, "npc.lua");
	lua_pcall(_L, 0, 0, 0);

	lua_getglobal(_L, "set_uid");
	lua_pushnumber(_L, _id);
	lua_pcall(_L, 1, 0, 0);

	lua_register(_L, "API_SendMessage", API_SendMessage);
	lua_register(_L, "API_get_x", API_get_x);
	lua_register(_L, "API_get_y", API_get_y);
}

void NPC::DoRandomMove()
{
	short x = _x;
	short y = _y;

	
	switch (rand()%4)
	{
	case 0:
		if (x < (W_WIDTH - 1)) x++; 
		_dir = RIGHT;
		break;
	case 1:
		if (x > 0) x--;
		_dir = LEFT;

		break;
	case 2:
		if (y < (W_HEIGHT - 1)) y++;
		_dir = DOWN;
		break;
	case 3:
		if(y > 0) y--;
		_dir = UP;
		break;
	default:
		break;
	}


	//TODO. 이동가능한지 아닌지 확인해야함->랜덤이동은 쨋든 그렇다.

	_x = x;
	_y = y;

	int preCol = _sectorCol;
	int preRow = _sectorRow;

	int curCol = _sectorCol = _x / SECTOR_SIZE;
	int curRow = _sectorRow = _y / SECTOR_SIZE;

	if (preCol != curCol || preRow != curRow) {
		SessionManager::sector[preCol][preRow].EraseObjectInSector(_id);
		SessionManager::sector[curCol][curRow].InsertObjectInSector(_id);

	}

}




