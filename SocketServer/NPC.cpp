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

	//���

}



void NPC::init(int x, int y, int visual)
{
	astar.init(x, y, 20);
	_id = MAX_USER + TotalNpcCount++;
	
	_state = ST_INGAME;
	//NPC�� �ϴ� 10�̷� �ϱ�
	//Ÿ�Ժ��� �޶���ϳ�??
	_damage = 10;

	_x = x;
	_y = y;
	_rangeX = x + 20;
	_rangeY = y + 20;

	_visual = visual;
	_hp = 100;

	//id���ƴ϶� type�� ����ϴ°ŷ� �ϴ� �ϰڴ�.
	sprintf_s(_name, "NPC%d", _id);


	switch (visual)
	{
	case PEACE_FIXED:
		_isMove = false;
		break;
	case PEACE_ROAMING:
		_isMove = true;
		break;
	case AGRO_FIXED:
		_isMove = false;

		break;
	case AGRO_ROAMING:
		_isMove = true;
		break;

	default:
		break;
	}

	_is_active = false;
	

	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;


	//��..�ֱ׷���..
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


	//TODO. �̵��������� �ƴ��� Ȯ���ؾ���->�����̵��� ¶�� �׷���.

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

void NPC::OnAttackSuccess(int type)
{
	//��� type�ʿ�����ѵ� �� �ϴ� ����
}

bool NPC::OnAttackReceived(int damage)
{

	//�ϴ� 10�� ������.
	std::cout << _id << "�� npc�� ������ ���ؼ� " << _hp << "���� ";

	_hp -= damage;
	std::cout << _hp << "�� �Ǿ���" << std::endl;


	if (_hp <= 0) {
		std::cout << _id <<" ���ʹ� �׾���." << std::endl;
		
		SessionManager::sector[_sectorCol][_sectorRow].EraseObjectInSector(_id);

		return true;
	}
	return false;
	


}





