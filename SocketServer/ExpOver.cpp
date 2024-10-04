#include "stdafx.h"
#include "ExpOver.h"


//Recv일때 생성되는 오버랩드 구조체
ExpOver::ExpOver()
{
	_wsabuf.len = BUF_SIZE;
	_wsabuf.buf = _io_buf;
	_comp_type = OP_RECV;
	ZeroMemory(&_over, sizeof(_over));
}

//Send할때 생성되는 오버랩드 구조체
ExpOver::ExpOver(char* packet)
{
	_wsabuf.len = packet[0];
	_wsabuf.buf = _io_buf;
	ZeroMemory(&_over, sizeof(_over));
	_comp_type = OP_SEND;
	memcpy(_io_buf, packet, packet[0]);
}

void ExpOver::SettingData(char* packet)
{
	_wsabuf.len = packet[0];
	_wsabuf.buf = _io_buf;
	ZeroMemory(&_over, sizeof(_over));
	_comp_type = OP_SEND;
	memcpy(_io_buf, packet, packet[0]);

}

