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

    // 서버
    HANDLE _iocp;

    WSADATA WSAData;
    SOCKADDR_IN server_addr;
    SOCKADDR_IN cl_addr;
    int addr_size;

    // 나중에 유니크포인터 사용하는거로 바꿔도될듯?
    SessionManager* _sessionMgr;
    PacketManager* _packetMgr;
    MapManager* _mapMgr;
    DBConnectionPool* _dbConnPool;

    concurrency::concurrent_priority_queue<TimerEvent> _timerQueue;

    // 정적 멤버 변수로 유일한 인스턴스 보관
    static Server* instance;

    // private 생성자
    Server();

    // private 복사 생성자와 대입 연산자 방지
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

public:
    ~Server();

    // 인스턴스를 반환하는 정적 메서드
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