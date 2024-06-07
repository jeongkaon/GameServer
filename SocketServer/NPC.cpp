#include "stdafx.h"
#include "NPC.h"
#include "SessionManager.h"

int NPC::TotalNpcCount = 0;
NPC::NPC()
{
	_id = MAX_USER + TotalNpcCount++;
	_state = ST_INGAME;
	_x = 0;
	_y = 0;

	//TODO. ���� visual �߰��ؾ��Ѵ�.
	_type = 0;		//�ϴ� �� 0���� ����
	_moveType = 0;	

	_is_active = false;//??

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
	_state = ST_INGAME;
	_x = x;
	_y = y;

	_type = 0;		//�ϴ� �� 0���� ����
	_moveType = 0;

	_is_active = false;//??

	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;

	SessionManager::sector[_sectorCol][_sectorRow].InsertObjectInSector(_id);

	sprintf_s(_name, "NPC%d", _id);

}


void NPC::init()
{
	std::cout << "helloword" << std::endl;
		
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


	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;




}


void NPC::BroadCastingInSection(void* buf)
{

}


