#include "stdafx.h"
#include "Session.h"

Session::Session()
{
	_id = -1;
	_socket = 0;
	_x = _y = 0;
	_name[0] = 0;
	_state = ST_FREE;
	_prevRemain = 0;

	_sectorCol = -1;
	_sectorRow = -1;

	_exp = 0;
	
	_level = 1;

	//레벨에 따라 공격력 달라진다.
	_damage = _level * 10;
	
	_hp = 100;
	_maxHp = 100;
}

void Session::init(void* p)
{
	GameData* data = (GameData*)p;
	
	strcpy_s(_name, data->user_name);

	_x = data->user_x;
	_y = data->user_y;
	_exp = data->user_exp;
	_hp = data->user_hp;
	_level = data->user_level;
	_visual = data->user_visual;
	
	_sectorCol = _x / SECTOR_SIZE;
	_sectorRow = _y / SECTOR_SIZE;



}

void Session::DoRecv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&_recvOver._over, sizeof(_recvOver._over));
	_recvOver._wsabuf.len = BUF_SIZE - _prevRemain;
	_recvOver._wsabuf.buf = _recvOver._send_buf + _prevRemain;

	WSARecv(_socket, &_recvOver._wsabuf, 1, 0, &recv_flag, &_recvOver._over, 0);

}

void Session::DoSend(void* packet)
{
	ExpOver* sdata = new ExpOver{ reinterpret_cast<char*>(packet) };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);

}

void Session::SendLoginPacket()
{
	SC_LOGIN_INFO_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
	p.x = _x;
	p.y = _y;
	p.visual = _visual;

	DoSend(&p);
}

void Session::SendMovePacket(char dir)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.dir = dir;
	p.x = _x;		

	p.y = _y;

	p.move_time = last_move_time;
	DoSend(&p);
}

void Session::SendMovePacket(int id, int x, int y, int time,char dir)
{

	SC_MOVE_OBJECT_PACKET p;
	p.id = id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.dir = dir;
	p.x = x;
	p.y = y;

	p.move_time = time;
	DoSend(&p);


}

void Session::SendAddPlayerPacket(int id, char* name, int x, int y, int visual)
{

	_vl.lock();
	_viewList.insert(id);
	_vl.unlock();

	SC_ADD_OBJECT_PACKET add_packet;
	add_packet.id = id;
	strcpy_s(add_packet.name, name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_OBJECT;
	add_packet.x = x;
	add_packet.y = y;
	add_packet.visual = visual;

	DoSend(&add_packet);
}

void Session::SendRemovePlayerPacket(int id)
{
	_vl.lock();
	if (_viewList.count(id)) {
		_viewList.erase(id);
	}
	else {
		_vl.unlock();
		return;
	}
	_vl.unlock();

	SC_REMOVE_OBJECT_PACKET p;
	p.id = id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	DoSend(&p);

}

void Session::SendChatPacket(int id, const char* mess)
{
	SC_CHAT_PACKET packet;
	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_CHAT;
	strcpy_s(packet.mess, mess);
	DoSend(&packet);
}

void Session::SendChoiceCharPacket()
{
	SC_CHOICECHAR_PACKET packet;
	packet.size = sizeof(SC_CHOICECHAR_PACKET);
	packet.type = SC_CHOICE_CHARACTER;
	DoSend(&packet);

}

void Session::SendAttackSuccessPakcet(int npcId, int damage)
{
	SC_SUC_ATTACK_PACKET packet;
	packet.size = sizeof(SC_SUC_ATTACK_PACKET);
	packet.type = SC_ATTACK_SUCCESS;
	packet.npcId = npcId;
	packet.damage = damage;
	DoSend(&packet);

}

void Session::SendPlayerGetExpAndLv(int npcId)
{
	SC_GET_EXP_PACKET packet;
	packet.size = sizeof(SC_GET_EXP_PACKET);
	packet.type = SC_GET_EXP;
	packet.exp = _exp;
	packet.npcId = npcId;
	packet.level = _level;
	DoSend(&packet);

}

void Session::SendReceivedDamage(int npcId,int damage)
{
	SC_PLYAER_ATTACKED_PACKET packet;
	packet.size = sizeof(SC_PLYAER_ATTACKED_PACKET);
	packet.type = SC_PLYAER_ATTACKED;
	packet.npcId = npcId;
	packet.damage = damage;
	DoSend(&packet);

}

bool Session::OnAttackReceived(int damage)
{
	
	_hp -= damage;

	if (_hp < 0) {
		_x = 5;
		_y = 5;
		_exp *= 0.5;

		_hp = 50;


		return true;
	}

	return false;

}

void Session::UpdatePlayerExpAndLevel(int visual, int npcId)
{

	if (visual == PEACE_FIXED) {
		_exp = _level * _level * 2;

	}
	else {
		_exp += _level * _level * 2 * 2;
	}
	switch (_exp / 100)
	{
	case LEVEL2:
		std::cout << "LEVEL2 업그레이드\n";
		_level = 2;
		break;
	case LEVEL3:
		std::cout << "LEVEL3 업그레이드\n";
		_level = 3;
		break;
	case LEVEL4:
		std::cout << "LEVEL4 업그레이드\n";
		_level = 4;
		break;
	case LEVEL5:
		std::cout << "LEVEL5 업그레이드\n";
		_level = 5;
		break;
	default:
		break;
	}

	//이거를 레벨이랑 경험치를 같이보내자
	SendPlayerGetExpAndLv(npcId);


}


