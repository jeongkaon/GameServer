#include "stdafx.h"
#include "Server.h"

#include "SessionManager.h"
#include "PacketHandler.h"
#include "MapManager.h"
#include "DBConnectionPool.h"
#include "MemoryPool.h"

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

	_memeoryPool = new MemoryPool(sizeof(ExpOver), 5000);

	_sessionMgr = new SessionManager();
	_sessionMgr->Init(_memeoryPool);

	_dbConnPool = new DBConnectionPool();
	_dbConnPool->Connect(10, L"2024_TF_GS");

	_packetMgr = new PacketHandler();
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
	AcceptEx(_serverSocket, _clientSocket, a_over._io_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	_timerThread = std::thread(&Server::Timer, this);

	for (int i = 0; i < _numThreads; ++i)
		_workerThread.emplace_back(&Server::Worker,10, this);



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

		TimerEvent ev;	
		auto current_time = chrono::system_clock::now();
		if (true == _timerQueue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				_timerQueue.push(ev);		
											
				this_thread::sleep_for(1ms);
				continue;
			}
			switch (ev.event_id) 
			{
			case EV_RANDOM_MOVE:
			{
				ExpOver* ov = _memeoryPool->allocate();
				ov->_comp_type = OP_NPC_MOVE;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;
			}
			case EV_AGRO_MOVE:
			{
				ExpOver* ov = _memeoryPool->allocate();
				ov->_comp_type = OP_NPC_AGRO_MOVE;

				ov->_ai_target_obj = ev.target_id;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;
			}
			case EV_RECOVER_HP:
			{
				ExpOver* ov = _memeoryPool->allocate();
				ov->_comp_type = OP_RECOVER_HP;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;

			}
			case EV_NPC_DIE:
			{
				ExpOver* ov = _memeoryPool->allocate();
				ov->_comp_type = OP_NPC_RESPAWN;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;
			}
			}
		
			continue;		
		}
		this_thread::sleep_for(1ms);  
	}

}
void Server::Worker(int tid)
{
	_threadId = tid;
	while (true) {
		DWORD io_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(_iocp, &io_byte, &key, &over, INFINITE);
		ExpOver* exOver = reinterpret_cast<ExpOver*>(over);


		if (FALSE == ret) {
			if (exOver->_comp_type == OP_ACCEPT) std::cout << "Accept Error";
			else {
				std::cout << "GQCS Error on client[" << key << "]\n";

				_sessionMgr->disconnect(static_cast<int>(key));
				if (exOver->_comp_type == OP_SEND) {
					_memeoryPool->deallocate(exOver);
				}
				continue;
			}
		}

		if ((0 == io_byte) && ((exOver->_comp_type == OP_RECV) || (exOver->_comp_type == OP_SEND))) {
			_sessionMgr->disconnect(static_cast<int>(key));
			if (exOver->_comp_type == OP_SEND) {
				_memeoryPool->deallocate(exOver);
			}
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
			}
			else {
				std::cout << "Max user exceeded.\n";

			}

			ZeroMemory(&a_over._over, sizeof(a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(_serverSocket, _clientSocket, a_over._io_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);
			break;

		}
		case OP_RECV: {
			int process_data = io_byte + static_cast<Session*>(_sessionMgr->objects[key])->_prevRemain;
			char* buf = exOver->_io_buf;
			PACKET_HEADER* headerInfo = reinterpret_cast<PACKET_HEADER*>(buf);
			while (process_data > 0) {
				if (headerInfo->size <= process_data) {
					_packetMgr->ProcessRecvPacket(static_cast<int>(key), buf, headerInfo->size,headerInfo->type);
					buf +=  headerInfo->size;
					process_data -= headerInfo->size;
				}
				else break;
			}
			static_cast<Session*>(_sessionMgr->objects[key])->_prevRemain = process_data;
			if (process_data != 0) {
				memcpy(exOver->_io_buf, buf, process_data);
			}
			static_cast<Session*>(_sessionMgr->objects[key])->DoRecv();
			break;

		}
		case OP_SEND: {
			
			_memeoryPool->deallocate(exOver);
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
			_memeoryPool->deallocate(exOver);
			break;

		}
		case OP_PLAYER_MOVE: {

			auto L = static_cast<NPC*>(_sessionMgr->objects[key])->_L;

			_sessionMgr->objects[key]->_sLock.lock();

			lua_getglobal(L, "event_player_move");			//이걸 호출해줘야한다.
			lua_pushnumber(L, exOver->_ai_target_obj);		//ai_target_obj가 호출했다.
			lua_pcall(L, 1, 0, 0);


			_sessionMgr->objects[key]->_sLock.unlock();

			_memeoryPool->deallocate(exOver);
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


			_memeoryPool->deallocate(exOver);
			break;
		}
		case OP_NPC_RESPAWN:
		{
			_sessionMgr->RespawnNPC(key);
			_memeoryPool->deallocate(exOver);
			break;

		}
		case OP_NPC_AGRO_MOVE:
		{

			_sessionMgr->objects[key]->_sLock.lock();

			if (_sessionMgr->objects[key]->_hp <= 0 ||
				_sessionMgr->NpcAstarMove(key, exOver->_ai_target_obj)==false) 
			{
				static_cast<NPC*>(_sessionMgr->objects[key])->_is_agro = false;
				_sessionMgr->objects[key]->_sLock.unlock();
				break;
			}
			auto L = static_cast<NPC*>(_sessionMgr->objects[key])->_L;

			lua_getglobal(L, "event_player_move");			//이걸 호출해줘야한다.
			lua_pushnumber(L, exOver->_ai_target_obj);		//ai_target_obj가 호출했다.
			lua_pcall(L, 1, 0, 0);


			_sessionMgr->objects[key]->_sLock.unlock();

			TimerEvent ev{ key, chrono::system_clock::now() + 1s, EV_AGRO_MOVE, exOver->_ai_target_obj };
			_timerQueue.push(ev);



			_memeoryPool->deallocate(exOver);
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

			ExpOver* ov = _memeoryPool->allocate();

			ov->_comp_type = OP_NPC_AGRO_MOVE;
			ov->_ai_target_obj = player;
			
			PostQueuedCompletionStatus(_iocp, 1, npc, &ov->_over);
		}

		break;
	}
	case AGRO_ROAMING:
	{
		if (_sessionMgr->NpcAgroActive(npc, player)) {

			bool old_state = false;
			if (false == atomic_compare_exchange_strong(&static_cast<NPC*>(_sessionMgr->objects[npc])->_is_agro, &old_state, true))
				return;

			ExpOver* ov = _memeoryPool->allocate();

			ov->_comp_type = OP_NPC_AGRO_MOVE;
			ov->_ai_target_obj = player;
			PostQueuedCompletionStatus(_iocp, 1, npc, &ov->_over);

		}
		else {

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

		break;
	}
		
	default:
		break;
	}



}

void Server::InputTimerEvent(TimerEvent* ev)
{
	_timerQueue.push(*ev);
}



