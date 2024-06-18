#include "stdafx.h"
#include "Server.h"

#include "SessionManager.h"
#include "PacketManager.h"
#include "MapManager.h"
#include "DBConnectionPool.h"

Server* Server::instance = nullptr;

Server::Server()
{

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
			case EV_AGRO_MOVE:
			{
				ExpOver* ov = new ExpOver;
				ov->_comp_type = OP_NPC_AGRO_MOVE;
				ov->_ai_target_obj = ev.target_id;
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

			if (static_cast<NPC*>(_sessionMgr->objects[key])->_is_agro) return;
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

				_sessionMgr->NpcRandomMove(static_cast<int>(key));

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

			auto L = static_cast<NPC*>(_sessionMgr->objects[key])->_L;

			_sessionMgr->objects[key]->_sLock.lock();

			lua_getglobal(L, "event_player_move");			//이걸 호출해줘야한다.
			lua_pushnumber(L, exOver->_ai_target_obj);		//ai_target_obj가 호출했다.
			lua_pcall(L, 1, 0, 0);


			_sessionMgr->objects[key]->_sLock.unlock();

			delete exOver;
			break;
		}
		case OP_RECOVER_HP:
		{
	
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
		case OP_NPC_AGRO_MOVE:
		{


			_sessionMgr->NpcAstarMove(key, exOver->_ai_target_obj);

	
			_sessionMgr->objects[key]->_sLock.lock();

			auto L = static_cast<NPC*>(_sessionMgr->objects[key])->_L;

			lua_getglobal(L, "event_player_move");			//이걸 호출해줘야한다.
			lua_pushnumber(L, exOver->_ai_target_obj);		//ai_target_obj가 호출했다.
			lua_pcall(L, 1, 0, 0);


			_sessionMgr->objects[key]->_sLock.unlock();
			TimerEvent ev{ key, chrono::system_clock::now() + 1s, EV_AGRO_MOVE, exOver->_ai_target_obj };
			_timerQueue.push(ev);



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
	switch (_sessionMgr->objects[npc]->_visual)
	{
	case PEACE_FIXED:
		return;
	case PEACE_ROAMING:
	{
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
		break;

	}
	case AGRO_FIXED:
	{
		if (_sessionMgr->NpcAgroActive(npc, player) && static_cast<NPC*>(_sessionMgr->objects[npc])->_is_agro==false) {
			bool old_state = false;
			if (false == atomic_compare_exchange_strong(&static_cast<NPC*>(_sessionMgr->objects[npc])->_is_agro, &old_state, true))
				return;

			ExpOver* exover = new ExpOver;
			exover->_comp_type = OP_NPC_AGRO_MOVE;
			exover->_ai_target_obj = player;
			PostQueuedCompletionStatus(_iocp, 1, npc, &exover->_over);
		}

		break;
	}
	case AGRO_ROAMING:
	{
		//어그로 래인지에 들어왔는지 확인
		if (_sessionMgr->NpcAgroActive(npc, player)) {

			bool old_state = false;
			if (false == atomic_compare_exchange_strong(&static_cast<NPC*>(_sessionMgr->objects[npc])->_is_agro, &old_state, true))
				return;

			ExpOver* exover = new ExpOver;
			exover->_comp_type = OP_NPC_AGRO_MOVE;
			exover->_ai_target_obj = player;
			PostQueuedCompletionStatus(_iocp, 1, npc, &exover->_over);

		}
		else {

			//랜덤이동
			bool old_state = false;
			if (false == atomic_compare_exchange_strong(&static_cast<NPC*>(_sessionMgr->objects[npc])->_is_active, &old_state, true))
				return;

			std::chrono::system_clock::time_point nowTime = chrono::system_clock::now();

			if (nowTime - static_cast<NPC*>(_sessionMgr->objects[npc])->wakeupTime < 3ms) {
				return;
			}

			//이게 문제가 아닌듯? 
			static_cast<NPC*>(_sessionMgr->objects[npc])->wakeupTime = nowTime;



			TimerEvent ev{ npc, chrono::system_clock::now(), EV_RANDOM_MOVE, 0 };
			_timerQueue.push(ev);

		}

		//랜덤이동
		break;
	}
		
	default:
		break;
	}



}

void Server::InputTimerEvent(TimerEvent* ev)
{
	_timerQueue.push(*ev);
	delete ev;
}



