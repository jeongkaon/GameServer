#include "stdafx.h"
#include "SessionManager.h"
#include "Server.h"
#include "LuaFunction.h"

using namespace std;

Sector SessionManager::sector[SECTOR_NUM][SECTOR_NUM]{};

SessionManager::SessionManager()
{
	server = Server::getInstance();

	for (int i = 0; i < MAX_USER; ++i) {
		objects[i] = new Session();
	}
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		objects[i] = new NPC();
	}

}
void SessionManager::Init()
{

	ifstream in{ "npc.txt",ios::binary };

	int temp;

	int i = 0;
	int j = 0;

	while (in >> temp) {
		if (temp > 5) {
			_npcInfo[i][j] = 0;

		}
		else {
			_npcInfo[i][j] = temp;

		}
		++j;
		if (j >= LIMIT_X) {
			++i;
			j = 0;
		}
		if (i >= LIMIT_X) break;
	}

	int id = MAX_USER;
	for (int i = 0; i < LIMIT_Y; ++i) {
		for (int j = 0; j < LIMIT_X; ++j) {
			if (_npcInfo[i][j] == 0) continue;

			int vis = _npcInfo[i][j] + 10;
			//test 한다.AGRO_FIXED 초록색 테스트
			//static_cast<NPC*>(objects[id++])->init(server->_mapMgr,j, i, vis);
			static_cast<NPC*>(objects[id++])->init(server->_mapMgr, j, i, AGRO_FIXED);

		}
	}

	std::cout << "NPC 초기화 테스트중...\n";

}

int SessionManager::AcceptClient(SOCKET& socket)
{
	int id = RetNewClientId();
	if (-1 != id) {
		{
			std::lock_guard<std::mutex> ll(static_cast<Session*>(objects[id])->_sLock);
			static_cast<Session*>(objects[id])->_state = ST_ALLOC;
		}

		 static_cast<Session*>(objects[id])->_x = 0;
		 static_cast<Session*>(objects[id])->_y = 0;
		 static_cast<Session*>(objects[id])->_id = id;
		 static_cast<Session*>(objects[id])->_name[0] = 0;
		 static_cast<Session*>(objects[id])->_prevRemain = 0;
		 static_cast<Session*>(objects[id])->_socket = socket;

		return id;
	}
	else {
		return -1;
	}
}

unordered_set<int> SessionManager::UpdateViewlistInSection(const int curCol, const int curRow, const int id)
{
	unordered_set<int> viewList;
	unordered_set<int> objs;
	for (int i = -1; i < 2; ++i) {
		if (curCol + i <0 || curCol + i >SECTOR_NUM) continue;
		for (int j = -1; j < 2; ++j) {
			if (curRow + j <0 || curRow + j >SECTOR_NUM) continue;
			sector[curCol + i][curRow + j].SetObjectList(objs);
			for (int clientId : objs) {
				if (objects[clientId]->_state != ST_INGAME) continue;
				if (objects[clientId]->_id == id) continue;
				if (false == CanSee(id, objects[clientId]->_id)) continue;
				viewList.insert(objects[clientId]->_id);
			}
		}
	}
	return viewList;

}
int SessionManager::RetNewClientId()
{

	for (int i = 0; i < MAX_USER; ++i) {
		std::lock_guard <std::mutex> ll{ objects[i]->_sLock };
		if (objects[i]->_state == ST_FREE)
			return i;
	}
	return -1;
}
int SessionManager::CheckLoginSession(int id)
{
	return LOGIN_OK;
}

void SessionManager::LoginSession(int id)
{
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	objects[id]->SendLoginPacket();

	int col = objects[id]->_sectorCol;
	int row = objects[id]->_sectorRow;

	sector[col][row].InsertObjectInSector(id);


	unordered_set<int> objs;
	for (int i = -1; i < 2; ++i) {
		if (col + i <0 || col + i >SECTOR_NUM) continue;

		for (int j = -1; j < 2; ++j) {
			if (row + j <0 || row + j >SECTOR_NUM) 	continue;
			sector[col + i][row + j].SetObjectList(objs);

			for (int clientId : objs) {
				{
					lock_guard<mutex> ll(objects[clientId]->_sLock);
					if (ST_INGAME != objects[clientId]->_state) continue;
				}
				if (objects[clientId]->_id == id) continue;
				if (false == CanSee(id, objects[clientId]->_id)) continue;

				if (clientId < MAX_USER) {
					objects[clientId]->SendAddPlayerPacket(
						id, objects[id]->_name, objects[id]->_x, objects[id]->_y, objects[id]->_visual);
				}
				else {
					
					server->WakeupNpc(clientId, id);

				}
				objects[id]->SendAddPlayerPacket(clientId, objects[clientId]->_name,
					objects[clientId]->_x, objects[clientId]->_y, objects[clientId]->_visual);



			}
		}
	}


}
void SessionManager::MoveSession(int id, CS_MOVE_PACKET* packet)
{

	int x = objects[id]->_x;
	int y = objects[id]->_y;
	char dir = packet->direction;

	int preCol = objects[id]->_sectorCol;
	int preRow = objects[id]->_sectorRow;

	int curCol = objects[id]->_sectorCol = x / SECTOR_SIZE;
	int curRow = objects[id]->_sectorRow = y / SECTOR_SIZE;

	if (curCol != preCol || preRow != curRow) {
		sector[preCol][preRow].EraseObjectInSector(objects[id]->_id);
		sector[curCol][curRow].InsertObjectInSector(objects[id]->_id);
	}
	
	objects[id]->_vl.lock();
	unordered_set<int> old_vlist = objects[id]->_viewList;
	objects[id]->_vl.unlock();

	unordered_set<int> new_viewlist = UpdateViewlistInSection(curCol, curRow, id);

	objects[id]->SendMovePacket(dir);

	for (int objId : new_viewlist) {
		if (objId < MAX_USER) {
			objects[objId]->_vl.lock();
			if (objects[objId]->_viewList.count(id)) {
				objects[objId]->_vl.unlock();
				objects[objId]->SendMovePacket(id, x, y, objects[id]->last_move_time,dir);
			}
			else {
				objects[objId]->_vl.unlock();
				objects[objId]->SendAddPlayerPacket(id, objects[id]->_name, 
					objects[id]->_x, objects[id]->_y, objects[id]->_visual);
			}
		}
		else {
			server->WakeupNpc(objects[objId]->_id, id);
		}

		if (old_vlist.count(objId) == 0) {
			objects[id]->SendAddPlayerPacket(objId, objects[objId]->_name,
				objects[objId]->_x, objects[objId]->_y, objects[objId]->_visual);
		}
	}


	for (int objId : old_vlist) {
		if (0 == new_viewlist.count(objId)) {
			objects[id]->SendRemovePlayerPacket(objId);
			if (objId < MAX_USER) objects[objId]->SendRemovePlayerPacket(id);
		}
	}


}
void SessionManager::CharChoiceSession(int id)
{
	static_cast<Session*>(objects[id])->SendChoiceCharPacket();
}
bool SessionManager::CanSee(int from, int to)
{
	if (abs(objects[from]->_x - objects[to]->_x) > VIEW_RANGE) return false;
	return abs(objects[from]->_y - objects[to]->_y) <= VIEW_RANGE;
}

void SessionManager::disconnect(int key)
{
	for (auto& pl : objects) {
		{
			lock_guard<mutex> ll(pl->_sLock);
			if (ST_INGAME != pl->_state) continue;
		}
		if (pl->_id == key) continue;
		pl->SendRemovePlayerPacket(key);
	}


	sector[objects[key]->_sectorCol][objects[key]->_sectorRow].EraseObjectInSector(key);
	closesocket(static_cast<Session*>(objects[key])->_socket);

	lock_guard<mutex> ll(static_cast<Session*>(objects[key])->_sLock);
	static_cast<Session*>(objects[key])->_state = ST_FREE;

}
void SessionManager::NpcRandomMove(int id)
{
	int Col = objects[id]->_sectorCol;
	int Row = objects[id]->_sectorRow;

	std::unordered_set<int> old_vl = UpdateViewlistInSection(Col, Row, id);

	static_cast<NPC*>(objects[id])->DoRandomMove();

	Col = objects[id]->_sectorCol;
	Row = objects[id]->_sectorRow;

	unordered_set<int> new_viewlist= UpdateViewlistInSection(Col, Row, id);

	for (auto pl : new_viewlist) {
		if (0 == old_vl.count(pl)) {
			objects[pl]->SendAddPlayerPacket(objects[id]->_id,objects[id]->_name, 
				objects[id]->_x, objects[id]->_y, objects[id]->_visual);
		}
		else {
			objects[pl]->SendMovePacket(objects[id]->_id, 
				objects[id]->_x, objects[id]->_y, objects[id]->last_move_time,objects[id]->_dir);

		}
	}
	for (auto pl : old_vl) {
		if (0 == new_viewlist.count(pl)) {
			objects[pl]->_vl.lock();
			if (0 != objects[pl]->_viewList.count(objects[id]->_id)) {
				objects[pl]->_vl.unlock();
				objects[pl]->SendRemovePlayerPacket(objects[id]->_id);
			}
			else {
				objects[pl]->_vl.unlock();
			}
		}
	}
}
bool SessionManager::NpcAstarMove(int id, int target)
{
	//여기서 거리를 계산하고 RAGNE보다 멀어졌으면 그만하는거로? 바로 리턴때리는거로?

	if (NpcAgroActive(id, target)==false)
	{
		return false;
	}

	int Col = objects[id]->_sectorCol;
	int Row = objects[id]->_sectorRow;

	std::unordered_set<int> old_vl = UpdateViewlistInSection(Col, Row, id);


	static_cast<NPC*>(objects[id])->DoAstarMove(objects[target]->_x,objects[target]->_y);

	Col = objects[id]->_sectorCol;
	Row = objects[id]->_sectorRow;

	unordered_set<int> new_viewlist= UpdateViewlistInSection(Col, Row, id);

	for (auto pl : new_viewlist) {
		if (0 == old_vl.count(pl)) {
			objects[pl]->SendAddPlayerPacket(objects[id]->_id, objects[id]->_name,
				objects[id]->_x, objects[id]->_y, objects[id]->_visual);
		}
		else {
			objects[pl]->SendMovePacket(objects[id]->_id,
				objects[id]->_x, objects[id]->_y, objects[id]->last_move_time, objects[id]->_dir);

		}
	}
	for (auto pl : old_vl) {
		if (0 == new_viewlist.count(pl)) {
			objects[pl]->_vl.lock();
			if (0 != objects[pl]->_viewList.count(objects[id]->_id)) {
				objects[pl]->_vl.unlock();
				objects[pl]->SendRemovePlayerPacket(objects[id]->_id);
			}
			else {
				objects[pl]->_vl.unlock();
			}
		}
	}


	return true;

}
void SessionManager::NpcAttackedMove(int id)
{
	int Col = objects[id]->_sectorCol;
	int Row = objects[id]->_sectorRow;

	std::unordered_set<int> old_vl = UpdateViewlistInSection(Col, Row, id);

	Col = objects[id]->_sectorCol;
	Row = objects[id]->_sectorRow;

	unordered_set<int> new_viewlist = UpdateViewlistInSection(Col, Row, id);

	for (auto pl : new_viewlist) {
		if (0 == old_vl.count(pl)) {
			objects[pl]->SendAddPlayerPacket(objects[id]->_id, objects[id]->_name,
				objects[id]->_x, objects[id]->_y, objects[id]->_visual);
		}
		else {
			objects[pl]->SendMovePacket(objects[id]->_id,
				objects[id]->_x, objects[id]->_y, objects[id]->last_move_time, objects[id]->_dir);

		}
	}
	for (auto pl : old_vl) {
		if (0 == new_viewlist.count(pl)) {
			objects[pl]->_vl.lock();
			if (0 != objects[pl]->_viewList.count(objects[id]->_id)) {
				objects[pl]->_vl.unlock();
				objects[pl]->SendRemovePlayerPacket(objects[id]->_id);
			}
			else {
				objects[pl]->_vl.unlock();
			}
		}
	}
}
bool SessionManager::NpcAgroActive(int npc, int plyaer)
{
	if(objects[npc]->_x > static_cast<NPC*>(objects[npc])->_rangeX) return false;
	if (objects[npc]->_y > static_cast<NPC*>(objects[npc])->_rangeY) return false;


	if (abs(objects[npc]->_x - objects[plyaer]->_x) >= AGRO_ACTIVE_RANGE) return false;
	return abs(objects[npc]->_y - objects[plyaer]->_y) < AGRO_ACTIVE_RANGE;
}



void SessionManager::SleepNPC(int id)
{
	if (static_cast<NPC*>(objects[id])->_is_active == false) return;
	bool oldState = true;
	atomic_compare_exchange_strong(&static_cast<NPC*>(objects[id])->_is_active, &oldState, false);

}
void SessionManager::AttackSessionToNPC(int id, char dir)
{
	objects[id]->_vl.lock();
	unordered_set<int> vlist = objects[id]->_viewList;
	objects[id]->_vl.unlock();

	for (int npcId : vlist)	{
		std::cout <<"viewlist에 들어온 npc :" << npcId << std::endl;
	}
	std::cout<<std::endl;

	switch (dir)
	{
	case LEFT:
	case RIGHT:
		for (int npcId : vlist) {
			if (abs(static_cast<int>(objects[npcId]->_x - objects[id]->_x)) > ATTACK_RANGE) {
				continue;
			}
			Attack(npcId, id);

		}
		break;
	case UP:
	case DOWN:
		for (int npcId : vlist) {
			if (abs(static_cast<int>(objects[npcId]->_x - objects[id]->_x)) > ATTACK_RANGE) {
				continue;
			}
			Attack(npcId, id);

		}
		break;
	case ALL: //4방향공격인데 지금 8방향으로 되어있음..->고쳣나?
		for(int npcId : vlist){
			if (abs(static_cast<int>(objects[npcId]->_x- objects[id]->_x)) > ATTACK_RANGE) {
				continue;
			}
			if (abs(static_cast<int>(objects[npcId]->_y- objects[id]->_y)) > ATTACK_RANGE) {
				continue;
			}

			if (abs(static_cast<int>(objects[npcId]->_y - objects[id]->_y)) <= ATTACK_RANGE &&
				(objects[npcId]->_x ==objects[id]->_x)) {
				Attack(npcId, id);

			}
			if (abs(static_cast<int>(objects[npcId]->_x - objects[id]->_x)) <= ATTACK_RANGE &&
				(objects[npcId]->_y == objects[id]->_y)) {
				Attack(npcId, id);

			}
		}

		break;
	default:
		break;
	}
}
void SessionManager::Attack(int npcId, int id)
{
	cout <<npcId <<"번의 NPC 공격성공\n";

	static_cast<Session*>(objects[id])->SendAttackSuccessPakcet(npcId, objects[id]->_damage);

	//몬스터 죽었을 때 
	if (objects[npcId]->OnAttackReceived(objects[id]->_damage, objects[id]->_dir)) {

		//죽었으면 경험치 줘야한다. 
		static_cast<Session*>(objects[id])->UpdatePlayerExpAndLevel(objects[npcId]->_visual, npcId);


		int Col = objects[npcId]->_sectorCol;
		int Row = objects[npcId]->_sectorRow;

		unordered_set<int> _viewlist = UpdateViewlistInSection(Col, Row, id);

		for (int ClidntId : _viewlist) {
			objects[ClidntId]->SendRemovePlayerPacket(objects[npcId]->_id);
		}
		objects[id]->SendRemovePlayerPacket(objects[npcId]->_id);

		//TODO. TEST용으로 5초로 변경함 -> 30초로 해놔야한다. 30초 후에 부활하는 거임 
		TimerEvent* dieev = new TimerEvent{ npcId, std::chrono::system_clock::now() + 5s, EV_NPC_DIE, 0 };
		server->InputTimerEvent(dieev);

	}
	else {
		NpcAttackedMove(npcId);
	}

	
	if (objects[npcId]->_visual == PEACE_FIXED || objects[npcId]->_visual == PEACE_ROAMING) return;;
	//이거 공격받으면 잠시 멈추는거임 
	TimerEvent* ev = new TimerEvent{ npcId,  std::chrono::system_clock::now() + 1s ,EV_ACTIVE_MOVE,0 };
	server->InputTimerEvent(ev);


}
void SessionManager::RespawnNPC(int npcId)
{
	static_cast<NPC*>(objects[npcId])->RecoverHP();

	int Col = objects[npcId]->_sectorCol;
	int Row = objects[npcId]->_sectorRow;

	sector[Col][Row].InsertObjectInSector(npcId);

	unordered_set<int> _viewlist = UpdateViewlistInSection(Col, Row, npcId);

	for (int ClidntId : _viewlist) {
		objects[ClidntId]->SendAddPlayerPacket(objects[npcId]->_id, objects[npcId]->_name,
			objects[npcId]->_x, objects[npcId]->_y, objects[npcId]->_visual);
	}
}
void SessionManager::BroadcastChatting(int id, int len, void* chat )
{
	SC_CHAT_PACKET packet;
	packet.size = len + 7;
	packet.type = SC_CHAT;
	packet.id = id;
	strncpy(packet.mess, (char*)chat, len);

	for (int i = 0; i < MAX_USER; ++i) {

		{
			lock_guard<mutex> ll(objects[i]->_sLock);
			if (ST_INGAME != objects[i]->_state) continue;
		}
		static_cast<Session*>(objects[i])->DoSend(&packet);
	}

}



