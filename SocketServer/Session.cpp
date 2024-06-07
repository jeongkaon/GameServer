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

void Session::SendMovePacket()
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;

	p.x = _x;		
	p.y = _y;

	p.move_time = last_move_time;
	DoSend(&p);
}

void Session::SendMovePacket(int id, int x, int y, int time)
{

	SC_MOVE_OBJECT_PACKET p;
	p.id = id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;

	p.x = x;
	p.y = y;

	p.move_time = time;
	DoSend(&p);


}

void Session::SendAddPlayerPacket(int id, char* name, int x, int y, int visual)
{
	//인자로 받은 id의 클라이언트가 추가되었다는 정보를 보낸다.
	//TODO. 다른 정보를 어떻게 받을것인지
	//그냥 *client를 받을것인지를 생각해봐야함->락걸어야할거같음..
	
	//뷰리스트에 추가해야한다.

	//만약 이 함수가 호출된건 해당 섹션에 있다는 이야기니까 뷰리스트에 추가된다.??
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
	//id의 클라는 지워야할 클라이언트이다.
	//해당 세션의 뷰리스트를 일단 락을 걸고 
	//뷰리스트에 없으면 안지워도되고, 만약 뷰리스트에 있으면 지워야한다.
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


