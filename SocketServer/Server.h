#pragma once
#include "ExpOver.h"

class SessionManager;
class PacketManager;
class MapManager;
class DBConnectionPool;
struct TimerEvent;

class Server
{
private:
    int _numThreads;

    std::thread _timerThread;
    std::vector<std::thread> _workerThread;
    ExpOver a_over;

    SOCKET _serverSocket;
    SOCKET _clientSocket;

    // 서버
    HANDLE _iocp;

    WSADATA WSAData;
    SOCKADDR_IN server_addr;
    SOCKADDR_IN cl_addr;
    int addr_size;

    // 나중에 유니크포인터 사용하는거로 바꿔도될듯?
    SessionManager* _sessionMgr;
    PacketManager* _packetMgr;
    DBConnectionPool* _dbConnPool;

    concurrency::concurrent_priority_queue<TimerEvent> _timerQueue;

    static Server* instance;

    Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

public:
    ~Server();
    //일단 여기로옮김
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
