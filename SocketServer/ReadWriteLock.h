#pragma once
#include "stdafx.h"

class ReadWriteLock
{
	std::atomic<short> _lockFlag = EMPTY_FLAG;
	int _writeCount = 0;    

	enum : short
	{
		ACQUIRE_TIMEOUT_TICK = 10000,       //최대로 기다려줄 tick
		MAX_SPIN_COUNT = 5000,              //스핀카운트를 최대 몇번돌건지
		WRITE_THREAD_MASK = 0xFF00,    //write카운터를 추출하기 위함
		READ_COUNT_MASK = 0x00FF,      //read 카운터를 추출하기 위함
		EMPTY_FLAG = 0x0000,
		WRITE_FLAG = 0xFF00

	};
public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

};

