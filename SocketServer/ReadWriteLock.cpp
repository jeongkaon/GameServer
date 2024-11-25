#include "stdafx.h"
#include "ReadWriteLock.h"

void ReadWriteLock::WriteLock()
{
	const int lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 8;
	//LThreadId를 어디서 받아와야하는지를 생각해보자.
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	const long beginTick = ::GetTickCount64();
	const short desired = ((LThreadId << 8) & WRITE_THREAD_MASK);
	while (true)
	{
		for (int spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			short expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired))
			{
				_writeCount++;
				return;
			}
		}
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			std::cout << "Lock Timeout\n";


		std::this_thread::yield();
	}

}

void ReadWriteLock::WriteUnlock()
{
	
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0) {
		std::cout << "Ivalid Unlock order\n";
	}

	const int lockCount = --_writeCount;
	if (lockCount == 0) {
		_lockFlag.store(EMPTY_FLAG);
	}


}

void ReadWriteLock::ReadLock()
{
	const int lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}
	const long beginTick = ::GetTickCount64();
	while (true)
	{
		for (int spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			int expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			std::cout << "Lock Timeout\n";

		std::this_thread::yield();
	}

}

void ReadWriteLock::ReadUnlock()
{
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		std::cout << "MULTIPLE_UNLOCK\n";


}
