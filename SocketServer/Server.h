#pragma once
#include "ExpOver.h"

class SessionManager;
class PacketManager;
class MapManager;
class DBConnectionPool;
class TimerEvent;

class Server
{
	int _numThreads;

	std::thread _timerThread;
	std::vector<std::thread> _workerThread;
	ExpOver a_over;

	SOCKET _serverSocket;
	SOCKET _clientSocket;

	//서버
	HANDLE _iocp;

	WSADATA WSAData;
	SOCKADDR_IN server_addr;
	SOCKADDR_IN cl_addr;
	int addr_size;

	//나중에 유니크포인터 사용하는거로 바꿔도될듯?
	SessionManager* _sessionMgr;
	PacketManager* _packetMgr;	
	MapManager* _mapMgr;		
	DBConnectionPool* _dbConnPool;
				
	concurrency::concurrent_priority_queue<TimerEvent> _timerQueue;


public:
	Server();

public:
	void Init();
	void Start();
	void Stop();

	void Worker();
	void Timer();

};


//이걸 어디다둬야하나..
//타이머스레드가 얘를 꺼내서 한개씩 이벤트를 활성화시킨다.
//어떤 obj가 언제, 무엇을, 누구에게 해야하는지를 저장하는 구조체
struct TimerEvent {		//타이머
	int obj_id;
	std::chrono::system_clock::time_point wakeup_time;
	EventType event_id;		//event종류->무엇을에 해당한다->EventType
	int target_id;


	constexpr bool operator < (const TimerEvent& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};