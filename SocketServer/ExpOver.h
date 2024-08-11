#pragma once

class ExpOver
{
public:
	WSAOVERLAPPED _over;		
	WSABUF _wsabuf;

	char _send_buf[BUF_SIZE];
	CompleteType _comp_type;

	int _ai_target_obj;			

public:
	ExpOver();
	ExpOver(char* packet);

};

