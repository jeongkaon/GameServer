#pragma once

class ExpOver
{
public:
	WSAOVERLAPPED _over;			//맨위에 놔야 얘가 클래스 주소가된다.
	WSABUF _wsabuf;

	char _send_buf[BUF_SIZE];
	CompleteType _comp_type;

	int _ai_target_obj;			

public:
	ExpOver();
	ExpOver(char* packet);

};

