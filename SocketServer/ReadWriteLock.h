#pragma once
#include "stdafx.h"

class ReadWriteLock
{
	std::atomic<short> _lockFlag = EMPTY_FLAG;
	int _writeCount = 0;    

	enum : short
	{
		ACQUIRE_TIMEOUT_TICK = 10000,       //�ִ�� ��ٷ��� tick
		MAX_SPIN_COUNT = 5000,              //����ī��Ʈ�� �ִ� ���������
		WRITE_THREAD_MASK = 0xFF00,    //writeī���͸� �����ϱ� ����
		READ_COUNT_MASK = 0x00FF,      //read ī���͸� �����ϱ� ����
		EMPTY_FLAG = 0x0000,
		WRITE_FLAG = 0xFF00

	};
public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

};

