#pragma once
#include "ExpOver.h"

class SessionManager;
class PacketManager;
class MapManager;
class DBConnectionPool;

class Server
{
	int _numThreads;

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
				
	

public:
	Server();

public:
	void Init();
	void Start();
	void Stop();

	void Worker();
};

