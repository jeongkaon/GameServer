#pragma once
#include "ExpOver.h"

class SessionManager;
class PacketManager;
class MapManager;
class DBConnectionPool;
class TimerEvent;

class Server
{
private:
    int _numThreads;

    std::thread _timerThread;
    std::vector<std::thread> _workerThread;
    ExpOver a_over;

    SOCKET _serverSocket;
    SOCKET _clientSocket;

    // ����
    HANDLE _iocp;

    WSADATA WSAData;
    SOCKADDR_IN server_addr;
    SOCKADDR_IN cl_addr;
    int addr_size;

    // ���߿� ����ũ������ ����ϴ°ŷ� �ٲ㵵�ɵ�?
    SessionManager* _sessionMgr;
    PacketManager* _packetMgr;
    MapManager* _mapMgr;
    DBConnectionPool* _dbConnPool;

    concurrency::concurrent_priority_queue<TimerEvent> _timerQueue;

    // ���� ��� ������ ������ �ν��Ͻ� ����
    static Server* instance;

    // private ������
    Server();

    // private ���� �����ڿ� ���� ������ ����
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

public:
    ~Server();

    // �ν��Ͻ��� ��ȯ�ϴ� ���� �޼���
    static Server* getInstance();

    void Init();
    void NpcInit();
    void Start();
    void Stop();

    void Worker();
    void Timer();

    int LuaGetX(int id); 
    int LuaGetY(int id); 
};
int API_get_x(lua_State* L);
int API_get_y(lua_State* L);