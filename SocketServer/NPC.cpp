#include "stdafx.h"
#include "NPC.h"
#include "SessionManager.h"
#include "MapManager.h"
#include "Sector.h"
#include "LuaFunction.h"


int NPC::TotalNpcCount = 0;
NPC::NPC()
{

	sprintf_s(_name, "NPC%d", _id);

}
NPC::NPC(int x, int y)
{


}



void NPC::init(MapManager* mgr, int x, int y, int visual)
{

	_mapMgr = mgr;
	_id = MAX_USER + TotalNpcCount++;
	_pathIndex = 0;
	_state = ST_INGAME;

	_damage = 10;
	_x = x;
	_y = y;
	_rangeX = x + 20;
	_rangeY = y + 20;

	_visual = visual;
	_hp = 100;

	sprintf_s(_name, "NPC%d", _id);
	

	switch (visual)
	{
	case PEACE_FIXED:
		_moveType = MOVE_FIXED;
		_monType = TYPE_PEACE;
		break;
	case PEACE_ROAMING:
		_moveType = MOVE_ROAMING;
		_monType = TYPE_PEACE;

		break;
	case AGRO_FIXED:
		_moveType = MOVE_FIXED;
		_monType = TYPE_AGRO;

		break;
	case AGRO_ROAMING:
		_moveType = MOVE_ROAMING;
		_monType = TYPE_AGRO;

		break;

	default:
		break;
	}

	//일단 처음은 끈다 -> 캐릭터가 깨웠을 때 활성화하는거임
	_is_active = false;
	_is_agro = false;


	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;

	SessionManager::sector[_sectorCol][_sectorRow].InsertObjectInSector(_id);

	_L = luaL_newstate();

	luaL_openlibs(_L);
	luaL_loadfile(_L, "npc.lua");
	lua_pcall(_L, 0, 0, 0);

	lua_getglobal(_L, "set_uid");
	lua_pushnumber(_L, _id);
	lua_pcall(_L, 1, 0, 0);

	lua_register(_L, "API_SendMessage", API_AttackMessage);
	lua_register(_L, "API_get_x", API_get_x);
	lua_register(_L, "API_get_y", API_get_y);
	lua_register(_L, "API_Active_Agro", API_Active_Agro);

	lua_register(_L, "API_MoveTo", API_MoveTo);

}


void NPC::DoRandomMove()
{
	short x = _x;
	short y = _y;

	
	switch (rand()%4)
	{
	case RIGHT:
		if (x < (W_WIDTH - 1)) x++; 
		_dir = RIGHT;
		break;
	case LEFT:
		if (x > 0) x--;
		_dir = LEFT;

		break;
	case DOWN:
		if (y < (W_HEIGHT - 1)) y++;
		_dir = DOWN;
		break;
	case UP:
		if(y > 0) y--;
		_dir = UP;
		break;
	default:
		break;
	}

	if (_mapMgr->IsCanGoCheck(_x, _y)) {
		_x = x;
		_y = y;

	}


	int preCol = _sectorCol;
	int preRow = _sectorRow;

	int curCol = _sectorCol = _x / SECTOR_SIZE;
	int curRow = _sectorRow = _y / SECTOR_SIZE;

	if (preCol != curCol || preRow != curRow) {
		SessionManager::sector[preCol][preRow].EraseObjectInSector(_id);
		SessionManager::sector[curCol][curRow].InsertObjectInSector(_id);

	}

}

void NPC::DoAstarMove(int desx ,int desy)
{
	
	astar.init(_x, _y, desx, desy);
	astar.FindPath(_mapMgr, &path);

	if (path.size() == 1) {
		_x = path[0].first;
		_y = path[0].second;

	}
	else {
		_x = path[1].first;
		_y = path[1].second;

	}

	int preCol = _sectorCol;
	int preRow = _sectorRow;

	int curCol = _sectorCol = _x / SECTOR_SIZE;
	int curRow = _sectorRow = _y / SECTOR_SIZE;

	if (preCol != curCol || preRow != curRow) {
		SessionManager::sector[preCol][preRow].EraseObjectInSector(_id);
		SessionManager::sector[curCol][curRow].InsertObjectInSector(_id);

	}


}

bool NPC::isCanGo(short x, short y)
{
	return false;
}



void NPC::OnAttackSuccess(int type)
{
}

bool NPC::OnAttackReceived(int damage, int dir)
{
	_hp -= damage;


	if (_hp <= 0) {
		std::cout << _id <<" 몬스터는 죽음" << std::endl;
		SessionManager::sector[_sectorCol][_sectorRow].EraseObjectInSector(_id);

		return true;
	}
	short x = _x;
	short y = _y;

	switch (dir)
	{
	case RIGHT:
		if (x < (W_WIDTH - 1)) x++;
		break;
	case LEFT:
		if (x > 0) x--;

		break;
	case DOWN:
		if (y < (W_HEIGHT - 1)) y++;
		break;
	case UP:
		if (y > 0) y--;
		_dir = UP;
		break;
	default:
		break;
	}
	if (_mapMgr->IsCanGoCheck(x, y)) {
		_x = x;
		_y = y;
	}
	return false;
	


}

void NPC::RecoverHP()
{
	_hp = 100;
}





