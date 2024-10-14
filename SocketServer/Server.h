#pragma once
#include "ExpOver.h"

class SessionManager;
class PacketHandler;
class MapManager;
class DBConnectionPool;
struct TimerEvent;
class MemoryPool;

class Server
{
private:
    int _numThreads;

    std::thread _timerThread;
    std::vector<std::thread> _workerThread;
    ExpOver a_over;

    SOCKET _serverSocket;
    SOCKET _clientSocket;

    HANDLE _iocp;

    WSADATA WSAData;
    SOCKADDR_IN server_addr;
    SOCKADDR_IN cl_addr;
    int addr_size;

    SessionManager* _sessionMgr;
    PacketHandler* _packetMgr;
    DBConnectionPool* _dbConnPool;

    MemoryPool* _memeoryPool;

    concurrency::concurrent_priority_queue<TimerEvent> _timerQueue;

    static Server* instance;

    Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

public:
    ~Server();
    MapManager* _mapMgr;

    static Server* getInstance();

    void Init();
    void Start();
    void Stop();

    void Worker();
    void Timer();


    int LuaGetX(int id); 
    int LuaGetY(int id); 

    void WakeupNpc(int npc, int player);
    void InputTimerEvent(TimerEvent* ev);
};
