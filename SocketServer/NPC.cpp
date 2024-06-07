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

	//TODO. 몬스터 visual 추가해야한다.
	_type = 0;		//일단 다 0으로 설정
	_moveType = 0;	

	_is_active = false;//??
	//위에는 이니셜라이즈 쓰자..

	_x = rand() % W_WIDTH;
	_y = rand() % W_HEIGHT;
	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;


	sprintf_s(_name, "NPC%d", _id);

}
NPC::NPC(int x, int y)
{
	_id = MAX_USER + TotalNpcCount++;
	_state = ST_INGAME;
	_x = x;
	_y = y;

	_type = 0;		//일단 다 0으로 설정
	_moveType = 0;

	_is_active = false;//??

	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;


	sprintf_s(_name, "NPC%d", _id);

}


void NPC::init()
{
	std::cout << "helloword" << std::endl;
		
}

void NPC::DoRandomMove()
{

}

void NPC::BroadCastingInSection(void* buf)
{

}


