#include "stdafx.h"
#include "Server.h"

#include "SessionManager.h"
#include "PacketManager.h"
#include "MapManager.h"
#include "DBConnectionPool.h"

Server* Server::instance = nullptr;

Server::Server()
{
	//여기다가 뭘 넣어야할지를 모르겠음..
}

Server* Server::getInstance()
{
	if (nullptr == instance) {
		instance = new Server();
	}
	return instance;
}

void Server::Init()
{
	_mapMgr = new MapManager();
	_mapMgr->InitMapInfo();
	//_mapMgr->InitNpcInfo();

	_sessionMgr = new SessionManager();
	_sessionMgr->Init();		//npc 정보를 받아야한다.

	_dbConnPool = new DBConnectionPool();
	_dbConnPool->Connect(10, L"2024_TF_GS");

	_packetMgr = new PacketManager();
	_packetMgr->Init(_sessionMgr, _mapMgr, _dbConnPool);

	//db테스트를 해보자
	// Read
	{
		//struct로 넘기는 방법 생각해보자

		DBConnection* dbConn = _dbConnPool->Pop();
		// 기존에 바인딩 된 정보 날림
		dbConn->Unbind();

		char level[20];;
		int hp;
		SQLLEN len = 0;
		SQLLEN len2 = 0;

		// 넘길 인자 바인딩
		dbConn->BindCol(1, level,20, &len);
		dbConn->BindCol(2, &hp, &len2);

		// SQL 실행
		// //SELECT user_id FROM user_table WHERE user_id = 1"
		//SQLExecDirect(hStmt, (SQLWCHAR*)L"SELECT * FROM game_server.dbo.[user_table];", SQL_NTS);
		dbConn->Excute(L"SELECT user_level FROM game_server.dbo.[game_data_table]");

		while (dbConn->Fetch())
		{
			cout << "name: " << level <<", level: "<<hp << endl;
		}

		//다썻으면 반납해야한다.

		_dbConnPool->Push(dbConn);
	}



	_numThreads = std::thread::hardware_concurrency();

}

void Server::Start()
{

	WSAStartup(MAKEWORD(2, 2), &WSAData);
	_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	addr_size = sizeof(cl_addr);

	bind(_serverSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(_serverSocket, SOMAXCONN);

	_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(_serverSocket), _iocp, 9999, 0);

	_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	a_over._comp_type = OP_ACCEPT;
	AcceptEx(_serverSocket, _clientSocket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	//타이머스레드 생성
	_timerThread = std::thread(&Server::Timer, this);

	for (int i = 0; i < _numThreads; ++i)
		_workerThread.emplace_back(&Server::Worker,this);



}



void Server::Stop()
{

	_timerThread.join();
	for (auto& th : _workerThread)
		th.join();

	delete _sessionMgr;
	delete _packetMgr;
	delete _mapMgr;
	delete _dbConnPool;

	closesocket(_serverSocket);
	WSACleanup();
}
void Server::Timer()
{
	while (true) {

		TimerEvent ev;		//그 구조체임 어디서 선언할지를 모르겟음..
		auto current_time = chrono::system_clock::now();
		if (true == _timerQueue.try_pop(ev)) {
			//top으로 엿보기로 바꾸자.
			if (ev.wakeup_time > current_time) {
				_timerQueue.push(ev);		// 최적화 필요
												// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(1ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.event_id) 
			{
			case EV_RANDOM_MOVE:
			{
				ExpOver* ov = new ExpOver;
				ov->_comp_type = OP_NPC_MOVE;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;
			}
			case EV_ACTIVE_MOVE:
			{
				ExpOver* ov = new ExpOver;
				ov->_comp_type = OP_NPC_MOVE_ACTIVE;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;

			}
			case EV_RECOVER_HP:
			{
				ExpOver* ov = new ExpOver;
				ov->_comp_type = OP_RECOVER_HP;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;

			}
			case EV_NPC_DIE:
			{
				ExpOver* ov = new ExpOver;
				ov->_comp_type = OP_NPC_RESPAWN;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;

			}
			}
		
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}

}
void Server::Worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(_iocp, &num_bytes, &key, &over, INFINITE);

		ExpOver* exOver = reinterpret_cast<ExpOver*>(over);
		if (FALSE == ret) {
			if (exOver->_comp_type == OP_ACCEPT) std::cout << "Accept Error";
			else {
				std::cout << "GQCS Error on client[" << key << "]\n";

				_sessionMgr->disconnect(static_cast<int>(key));
				if (exOver->_comp_type == OP_SEND) delete exOver;
				continue;
			}
		}

		if ((0 == num_bytes) && ((exOver->_comp_type == OP_RECV) || (exOver->_comp_type == OP_SEND))) {
			_sessionMgr->disconnect(static_cast<int>(key));
			if (exOver->_comp_type == OP_SEND) delete exOver;
			continue;
		}
		switch (exOver->_comp_type) {

		case OP_ACCEPT: {
			int clientId = _sessionMgr->AcceptClient(_clientSocket);

			if (-1 != clientId) {
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(_clientSocket),
					_iocp, clientId, 0);
				static_cast<Session*>(_sessionMgr->objects[clientId])->DoRecv();
				_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				std::cout << "User Accept.\n";
			}
			else {
				std::cout << "Max user exceeded.\n";

			}

			ZeroMemory(&a_over._over, sizeof(a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(_serverSocket, _clientSocket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);
			break;

		}
		case OP_RECV: {
			int remain_data = num_bytes + static_cast<Session*>(_sessionMgr->objects[key])->_prevRemain;
			char* p = exOver->_send_buf;
			while (remain_data > 0) {
				short* pSize = (short*)&p[0];
				int packet_size = *pSize;


				if (packet_size <= remain_data) {
					_packetMgr->ProcessRecvPacket(static_cast<int>(key), p, packet_size);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			static_cast<Session*>(_sessionMgr->objects[key])->_prevRemain = remain_data;
			if (remain_data > 0) {
				memcpy(exOver->_send_buf, p, remain_data);
			}
			static_cast<Session*>(_sessionMgr->objects[key])->DoRecv();
			break;

		}
		case OP_SEND: {
			delete exOver;
			break;
		}
		case OP_NPC_MOVE: {

			//move type이 roaming인 애들만 움직인다.->애초에 fix인애들한테는 이거 안들어옴
			//20x20에서 자유롭게 이동
			//장애물은 a* 길찾기로 이동한다..->이걸해결해야함

			bool keep_alive = false;

			int col = _sessionMgr->objects[key]->_sectorCol;
			int row = _sessionMgr->objects[key]->_sectorRow;
			
			unordered_set<int> objs; 
			for (int i = -1; i < 2; ++i) {
				if (col + i <0 || col + i >SECTOR_NUM) continue;
				for (int j = -1; j < 2; ++j) {
					if (row + j <0 || row + j >SECTOR_NUM) continue;
					SessionManager::sector[col + i][row + j].SetObjectList(objs);
					for (int clientId : objs) {
						if (_sessionMgr->objects[clientId]->_state != ST_INGAME) continue;
						if (clientId >= MAX_USER) continue;
						if (_sessionMgr->CanSee(static_cast<int>(key), clientId)) {
							keep_alive = true;
							break;
						}
					}
				}
			}

			if (true == keep_alive) {
				if (static_cast<NPC*>(_sessionMgr->objects[static_cast<int>(key)])->_moveType == MOVE_FIXED) break;

				//_sessionMgr->NpcRandomMove(static_cast<int>(key));
				_sessionMgr->NpcAstarMove(static_cast<int>(key));

				TimerEvent ev{ key, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
				_timerQueue.push(ev);
			}
			else {
				static_cast<NPC*>(_sessionMgr->objects[key])->_is_active = false;
			}
			delete exOver;
			break;

		}
		case OP_PLAYER_MOVE: {
			_sessionMgr->objects[key]->_sLock.lock();

			auto L = static_cast<NPC*>(_sessionMgr->objects[key])->_L;

			lua_getglobal(L, "event_player_move");			//이걸 호출해줘야한다.
			//lua_getglobal(L, "event_move_to_player");			//이걸 호출해줘야한다.
			lua_pushnumber(L, exOver->_ai_target_obj);		//ai_target_obj가 호출했다.
			lua_pcall(L, 1, 0, 0);
			//lua_pop(L, 1);
			_sessionMgr->objects[key]->_sLock.unlock();

			delete exOver;
			break;
		}
		case OP_NPC_MOVE_ACTIVE: {
			//TODO. 수정할거 있ㅇ다. 여러번때리면 여러번 추가되서 이상해짐 개빨리 움직임
			//이부분지워버릴까..
			std::cout << key << "NPC 다시 움직임 활성화됨\n";
			//static_cast<NPC*>(_sessionMgr->objects[static_cast<int>(key)])->_isMove = true;
			TimerEvent ev{ key, chrono::system_clock::now() + 3s, EV_RANDOM_MOVE, 0 };
			_timerQueue.push(ev);

			delete exOver;
			break;
		}
		case OP_RECOVER_HP:
		{
			//std::cout << key << "PLAYER의 체력 10%회복되는 타이머발동~\n";

			//TODO. max체력 안넘게 수정해야한다.
			//일단 현재의 hp의 10프로 회복하는거로함 

			//hp가 max일때도 나가야할텐디..?
			if (_sessionMgr->objects[static_cast<int>(key)]->_state != ST_INGAME)
			{
				return;
			}

			int hp = _sessionMgr->objects[static_cast<int>(key)]->_hp;

			_sessionMgr->objects[static_cast<int>(key)]->_hp += (hp * 0.1);

			TimerEvent ev{ key, chrono::system_clock::now() + 5s, EV_RECOVER_HP, 0 };
			_timerQueue.push(ev);


			delete exOver;
			break;
		}
		case OP_NPC_RESPAWN:
		{
			_sessionMgr->RespawnNPC(key);
			std::cout << key << "번째 NPC 부활" << std::endl;
			delete exOver;
			break;

		}
		}

	}
}


int Server::LuaGetX(int id)
{
	return _sessionMgr->objects[id]->_x;
}

int Server::LuaGetY(int id)
{
	return _sessionMgr->objects[id]->_y;
}

void Server::WakeupNpc(int npc, int player)
{
	//움직임 타입이 고정이면 안움직이니까 고정하자.
	if (static_cast<NPC*>(_sessionMgr->objects[npc])->_moveType == MOVE_FIXED) return;
	if (static_cast<NPC*>(_sessionMgr->objects[npc])->_is_active) return;

	//어그로구현
	// 1. x,y빼서 5안에 들어오면 11x11영역에 들어온거임
	// -> 영역안에 들어왔으면 쫒아오게 해야한다.
	// -> target_id 써서 루아쓰면될듯??
	if (static_cast<NPC*>(_sessionMgr->objects[npc])->_monType == TYPE_AGRO) {
		//타입어그로면 어쩌구저쩌구해야한다.
		//어그로일때만 루아할까??

	}
	else {
		//TODO. 이거 오버헤드가 엄청크다. 이 npc가 ai로 스크립트로 돌아가는건지 아닌지 flag을 둬
		//고정인거는 위에서 걸렀다.
		//이건 루아에서 해줄거 넣어주는거고 스크립트로 돌아가는것만 post해줘야한다->오버헤드가커서
		ExpOver* exover = new ExpOver;
		exover->_comp_type = OP_PLAYER_MOVE;
		exover->_ai_target_obj = player;
		PostQueuedCompletionStatus(_iocp, 1, npc, &exover->_over);

	}



	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&static_cast<NPC*>(_sessionMgr->objects[npc])->_is_active, &old_state, true))
		return;


	std::chrono::system_clock::time_point nowTime = chrono::system_clock::now();
	if (nowTime - static_cast<NPC*>(_sessionMgr->objects[npc])->wakeupTime < 3ms) {
		return;
	}
	static_cast<NPC*>(_sessionMgr->objects[npc])->wakeupTime = nowTime;

	TimerEvent ev{ npc, chrono::system_clock::now(), EV_RANDOM_MOVE, 0 };
	_timerQueue.push(ev);

}

void Server::InputTimerEvent(TimerEvent* ev)
{
	_timerQueue.push(*ev);
	delete ev;
}



