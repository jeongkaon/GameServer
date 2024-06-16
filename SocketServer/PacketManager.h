#pragma once
class SessionManager;
class MapManager;
class DBConnectionPool;

class PacketManager
{
	typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(int, char*, int);
	std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> _recvFuntionMap;

	SessionManager* _sessionMgr;
	MapManager* _mapMgr;
	DBConnectionPool* _dbConnPool;


public:

	void Init(SessionManager* sessionMgr, MapManager* mapMgr, DBConnectionPool* dbConnPool);

	void ProcessRecvPacket(int id, char* buf, int copySize);
	void ProcessRegistPacket(int id, char* buf, int copySize);
	void ProcessChoiceCharactertPacket(int id, char* buf, int copySize);
	void ProcessLoginPacket(int id, char* buf, int copySize);
	void ProcessStressLoginPacket(int id, char* buf, int copySize);

	void ProcessMovePacket(int id, char* buf, int copySize);
	void ProcessChattingPacket(int id, char* buf, int copySize);
	void ProcessAttackPacket(int id, char* buf, int copySize);
	void ProcessTeleportPacket(int id, char* buf, int copySize);
	void ProcessLogoutPacket(int id, char* buf, int copySize);

	//DB관련작업
	int CheckUserInDB(const char* name, GameData* data);
	bool AddUSerInDB(const char* name);
	bool AddUSerInDB(const GameData* gameData);

	bool AddVisualInfoInDB(const char* name, int visual);

};

