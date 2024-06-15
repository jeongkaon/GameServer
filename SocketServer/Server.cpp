#include "stdafx.h"
#include "Server.h"

#include "SessionManager.h"
#include "PacketManager.h"
#include "MapManager.h"
#include "DBConnectionPool.h"

Server* Server::instance = nullptr;

Server::Server()
{
	//����ٰ� �� �־�������� �𸣰���..
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
	_sessionMgr->Init();		//npc ������ �޾ƾ��Ѵ�.

	_dbConnPool = new DBConnectionPool();
	_dbConnPool->Connect(10, L"2024_TF_GS");

	_packetMgr = new PacketManager();
	_packetMgr->Init(_sessionMgr, _mapMgr, _dbConnPool);

	//db�׽�Ʈ�� �غ���
	// Read
	{
		//struct�� �ѱ�� ��� �����غ���

		DBConnection* dbConn = _dbConnPool->Pop();
		// ������ ���ε� �� ���� ����
		dbConn->Unbind();

		char level[20];;
		int hp;
		SQLLEN len = 0;
		SQLLEN len2 = 0;

		// �ѱ� ���� ���ε�
		dbConn->BindCol(1, level,20, &len);
		dbConn->BindCol(2, &hp, &len2);

		// SQL ����
		// //SELECT user_id FROM user_table WHERE user_id = 1"
		//SQLExecDirect(hStmt, (SQLWCHAR*)L"SELECT * FROM game_server.dbo.[user_table];", SQL_NTS);
		dbConn->Excute(L"SELECT user_level FROM game_server.dbo.[game_data_table]");

		while (dbConn->Fetch())
		{
			cout << "name: " << level <<", level: "<<hp << endl;
		}

		//�ٛ����� �ݳ��ؾ��Ѵ�.

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

	//Ÿ�̸ӽ����� ����
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

		TimerEvent ev;		//�� ����ü�� ��� ���������� �𸣰���..
		auto current_time = chrono::system_clock::now();
		if (true == _timerQueue.try_pop(ev)) {
			//top���� ������� �ٲ���.
			if (ev.wakeup_time > current_time) {
				_timerQueue.push(ev);		// ����ȭ �ʿ�
												// timer_queue�� �ٽ� ���� �ʰ� ó���ؾ� �Ѵ�.
				this_thread::sleep_for(1ms);  // ����ð��� ���� �ȵǾ����Ƿ� ��� ���
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
		
			continue;		// ��� ���� �۾� ������
		}
		this_thread::sleep_for(1ms);   // timer_queue�� ��� ������ ��� ��ٷȴٰ� �ٽ� ����
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

			//move type�� roaming�� �ֵ鸸 �����δ�.->���ʿ� fix�ξֵ����״� �̰� �ȵ���
			//20x20���� �����Ӱ� �̵�
			//��ֹ��� a* ��ã��� �̵��Ѵ�..->�̰��ذ��ؾ���

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

			lua_getglobal(L, "event_player_move");			//�̰� ȣ��������Ѵ�.
			//lua_getglobal(L, "event_move_to_player");			//�̰� ȣ��������Ѵ�.
			lua_pushnumber(L, exOver->_ai_target_obj);		//ai_target_obj�� ȣ���ߴ�.
			lua_pcall(L, 1, 0, 0);
			//lua_pop(L, 1);
			_sessionMgr->objects[key]->_sLock.unlock();

			delete exOver;
			break;
		}
		case OP_NPC_MOVE_ACTIVE: {
			//TODO. �����Ұ� �֤���. ������������ ������ �߰��Ǽ� �̻����� ������ ������
			//�̺κ�����������..
			std::cout << key << "NPC �ٽ� ������ Ȱ��ȭ��\n";
			//static_cast<NPC*>(_sessionMgr->objects[static_cast<int>(key)])->_isMove = true;
			TimerEvent ev{ key, chrono::system_clock::now() + 3s, EV_RANDOM_MOVE, 0 };
			_timerQueue.push(ev);

			delete exOver;
			break;
		}
		case OP_RECOVER_HP:
		{
			//std::cout << key << "PLAYER�� ü�� 10%ȸ���Ǵ� Ÿ�̸ӹߵ�~\n";

			//TODO. maxü�� �ȳѰ� �����ؾ��Ѵ�.
			//�ϴ� ������ hp�� 10���� ȸ���ϴ°ŷ��� 

			//hp�� max�϶��� ���������ٵ�..?
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
			std::cout << key << "��° NPC ��Ȱ" << std::endl;
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
	//������ Ÿ���� �����̸� �ȿ����̴ϱ� ��������.
	if (static_cast<NPC*>(_sessionMgr->objects[npc])->_moveType == MOVE_FIXED) return;
	if (static_cast<NPC*>(_sessionMgr->objects[npc])->_is_active) return;

	//��׷α���
	// 1. x,y���� 5�ȿ� ������ 11x11������ ���°���
	// -> �����ȿ� �������� �i�ƿ��� �ؾ��Ѵ�.
	// -> target_id �Ἥ ��ƾ���ɵ�??
	if (static_cast<NPC*>(_sessionMgr->objects[npc])->_monType == TYPE_AGRO) {
		//Ÿ�Ծ�׷θ� ��¼����¼���ؾ��Ѵ�.
		//��׷��϶��� ����ұ�??

	}
	else {
		//TODO. �̰� ������尡 ��ûũ��. �� npc�� ai�� ��ũ��Ʈ�� ���ư��°��� �ƴ��� flag�� ��
		//�����ΰŴ� ������ �ɷ���.
		//�̰� ��ƿ��� ���ٰ� �־��ִ°Ű� ��ũ��Ʈ�� ���ư��°͸� post������Ѵ�->������尡Ŀ��
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



