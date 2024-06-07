#include "stdafx.h"
#include "Server.h"

#include "SessionManager.h"
#include "PacketManager.h"
#include "MapManager.h"
#include "DBConnectionPool.h"

Server::Server()
{
	//여기다가 뭘 넣어야할지를 모르겠음..
}

void Server::Init()
{
	_mapMgr = new MapManager();
	_mapMgr->Init();

	_sessionMgr = new SessionManager();
	_sessionMgr->Init();

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
			if (ev.wakeup_time > current_time) {
				_timerQueue.push(ev);		// 최적화 필요
												// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(1ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.event_id) {
			case EV_RANDOM_MOVE:
				ExpOver* ov = new ExpOver;
				ov->_comp_type = OP_NPC_MOVE;
				PostQueuedCompletionStatus(_iocp, 1, ev.obj_id, &ov->_over);
				break;
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

		//&over의 주소로 들어온 클라이언트 구분한다.
		ExpOver* exOver = reinterpret_cast<ExpOver*>(over);
		if (FALSE == ret) {
			if (exOver->_comp_type == OP_ACCEPT) std::cout << "Accept Error";
			else {
				std::cout << "GQCS Error on client[" << key << "]\n";

				//디스커넥트는 세션매니저가 가지고 있어야할듯??><
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
			//어떤 클라이언트가 send했다. 서버는 그 패킷을 recv하는 상황.
			//패킷재조립이 필요함-> 일단 교수님 코드로해

			//위에서 이미 누가 보냈는지 확인함
			int remain_data = num_bytes + static_cast<Session*>(_sessionMgr->objects[key])->_prevRemain;
			char* p = exOver->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
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
		case OP_NPC_MOVE: {
			bool keep_alive = false;
			for (int j = 0; j < MAX_USER; ++j) {
				if (_sessionMgr->objects[j]->_state != ST_INGAME) continue;
				if (_sessionMgr->CanSee(static_cast<int>(key), j)) {
					keep_alive = true;
					break;
				}
			}
		
			if (true == keep_alive) {
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
			break;
		case OP_SEND:
			delete exOver;
			break;

		}
	}
}


