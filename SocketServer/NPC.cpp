#include "stdafx.h"
#include "NPC.h"
#include "SessionManager.h"
#include "Sector.h"
#include "LuaFunction.h"


int NPC::TotalNpcCount = 0;
NPC::NPC()
{
	_id = MAX_USER + TotalNpcCount++;
	_state = ST_FREE;
	_x = 0;
	_y = 0;

	//TODO. ���� visual �߰��ؾ��Ѵ�.
	_type = 0;		//�ϴ� �� 0���� ����
	_moveType = 0;	

	_is_active = false;//??

	//TODO. �ʱ���ġ �ʿ��� ��������.
	_x = rand() % W_WIDTH;
	_y = rand() % W_HEIGHT;

	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;

	//���ǿ� �߰��ؾ��Ѵ�.
	SessionManager::sector[_sectorCol][_sectorRow].InsertObjectInSector(_id);

	sprintf_s(_name, "NPC%d", _id);

}
NPC::NPC(int x, int y)
{
	_id = MAX_USER + TotalNpcCount++;
	_state = ST_FREE;
	_x = x;
	_y = y;

	_type = 0;		//�ϴ� �� 0���� ����
	_moveType = 0;

	_is_active = false;//??

	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;


	sprintf_s(_name, "NPC%d", _id);

	//���

}


void NPC::init()
{
	//TODO.�ʱ���ġ�� �ٲ����. -> �����ڿ� �ִ°� �Űܿ���!
	_state = ST_INGAME;
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

	switch (rand() % 4) {
	case 0: if (x < (W_WIDTH - 1)) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < (W_HEIGHT - 1)) y++; break;
	case 3:if (y > 0) y--; break;
	}

	//TODO. �̵��������� �ƴ��� Ȯ���ؾ���
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




