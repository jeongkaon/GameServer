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
				
	

public:
	Server();

public:
	void Init();
	void Start();
	void Stop();

	void Worker();
};

