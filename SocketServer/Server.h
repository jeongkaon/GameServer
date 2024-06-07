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

	//����
	HANDLE _iocp;

	WSADATA WSAData;
	SOCKADDR_IN server_addr;
	SOCKADDR_IN cl_addr;
	int addr_size;

	//���߿� ����ũ������ ����ϴ°ŷ� �ٲ㵵�ɵ�?
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


//�̰� ���ٵ־��ϳ�..
//Ÿ�̸ӽ����尡 �긦 ������ �Ѱ��� �̺�Ʈ�� Ȱ��ȭ��Ų��.
//� obj�� ����, ������, �������� �ؾ��ϴ����� �����ϴ� ����ü
struct TimerEvent {		//Ÿ�̸�
	int obj_id;
	std::chrono::system_clock::time_point wakeup_time;
	EventType event_id;		//event����->�������� �ش��Ѵ�->EventType
	int target_id;


	constexpr bool operator < (const TimerEvent& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};