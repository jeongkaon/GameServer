#include "stdafx.h"
#include "SessionManager.h"
#include "Server.h"
#include "LuaFunction.h"

using namespace std;

Sector SessionManager::sector[SECTOR_NUM][SECTOR_NUM]{};

SessionManager::SessionManager()
{
	std::cout << "herel!" << std::endl;
	server = Server::getInstance();

	//npc�̴ϼȶ������ϱ�
	for (int i = 0; i < MAX_USER; ++i) {
		objects[i] = new Session();
	}
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		objects[i] = new NPC();
	}

}
void SessionManager::Init()
{

	//NPC init�ϴºκ�
	ifstream in{ "npc.txt",ios::binary };

	int temp;

	int i = 0;
	int j = 0;



///temp�� -�� ���µ�??? �̻��ѵ�??
	while (in >> temp) {
		_npcInfo[i][j] = temp;
		++j;
		if (j >= 150) {
			++i;
			j = 0;
		}
		if (i >= 150) break;
	}

	int id = MAX_USER;
	for (int i = 0; i < 150; ++i) {
		for (int j = 0; j < 150; ++j) {
			if (_npcInfo[i][j] == 0) continue;

			static_cast<NPC*>(objects[id++])->init(j, i, _npcInfo[i][j]);
		}
	}

	std::cout << "NPC �ʱ�ȭ �׽�Ʈ��...\n";

}

int SessionManager::AcceptClient(SOCKET& socket)
{
	//player�϶� �������
	
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


int SessionManager::RetNewClientId()
{

	for (int i = 0; i < MAX_USER; ++i) {
		std::lock_guard <std::mutex> ll{ objects[i]->_sLock };
		//array�ϱ� ���ڸ� ã�Ƽ� �־��ִ°���.
		if (objects[i]->_state == ST_FREE)
			return i;
	}
	return -1;
}

void SessionManager::SetUserGameData(int id, GameData& data)
{
	//TODO.�α����Ҷ� ���Լ� ���ľ���!
	if (id > MAX_USER) return;
	
	strcpy_s(objects[id]->_name, data.user_name);

}

//�α��ο�û ���°� üũ�ؾ��Ѵ�.
int SessionManager::CheckLoginSession(int id)
{
	//error������ �����ؾ��Ѵ�.

	return LOGIN_OK;
}

void SessionManager::LoginSession(int id)
{
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	//������ �α��� ��Ŷ�� ������.
	objects[id]->SendLoginPacket();

	//�����ʱ�ȭ
	int col = objects[id]->_sectorCol;
	int row = objects[id]->_sectorRow;

	sector[col][row].InsertObjectInSector(id);

	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

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

				//�÷��̾��϶�
				if (clientId < MAX_USER) {
					objects[clientId]->SendAddPlayerPacket(
						id, objects[id]->_name, objects[id]->_x, objects[id]->_y, objects[id]->_visual);
				}
				else {
					//npc�϶�
					//Wakeup�������
					server->WakeupNpc(clientId, id);

				}
				objects[id]->SendAddPlayerPacket(clientId, objects[clientId]->_name,
					objects[clientId]->_x, objects[clientId]->_y, objects[clientId]->_visual);



			}
		}
	}

	//TODO.���⸦ ����ؾ��� -> ȸ���ϴ°Ÿ� �������� �ؾ������� �����غ����Ѵ�
	TimerEvent* ev = new TimerEvent{ id,  std::chrono::system_clock::now() + 1s ,EV_RECOVER_HP,0 };
	server->InputTimerEvent(ev);

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

	unordered_set<int> new_viewlist;
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
				new_viewlist.insert(objects[clientId]->_id);
			}
		}
	}

	objects[id]->SendMovePacket(dir);

	for (int clientId : new_viewlist) {
		//�÷��̾��
		if (clientId < MAX_USER) {
			objects[clientId]->_vl.lock();
			if (objects[clientId]->_viewList.count(id)) {
				objects[clientId]->_vl.unlock();
				objects[clientId]->SendMovePacket(id, x, y, objects[id]->last_move_time,dir);

			}
			else {
				objects[clientId]->_vl.unlock();
				objects[clientId]->SendAddPlayerPacket(id, objects[id]->_name, 
					objects[id]->_x, objects[id]->_y, objects[id]->_visual);
					
			}
		}
		else {
			//�����
			server->WakeupNpc(objects[clientId]->_id, id);
		}

		if (old_vlist.count(clientId) == 0) {
			objects[id]->SendAddPlayerPacket(clientId, objects[clientId]->_name,
				objects[clientId]->_x, objects[clientId]->_y, objects[clientId]->_visual);

		}

	}


	for (int clientId : old_vlist) {
		if (0 == new_viewlist.count(clientId)) {
			objects[id]->SendRemovePlayerPacket(clientId);
			if (clientId < MAX_USER) objects[clientId]->SendRemovePlayerPacket(id);
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
	closesocket(static_cast<Session*>(objects[key])->_socket);

	lock_guard<mutex> ll(static_cast<Session*>(objects[key])->_sLock);
	static_cast<Session*>(objects[key])->_state = ST_FREE;

}
void SessionManager::NpcRandomMove(int id)
{
	int Col = objects[id]->_sectorCol;
	int Row = objects[id]->_sectorRow;

	std::unordered_set<int> old_vl;
	std::unordered_set<int> objs;

	for (int i = -1; i < 1; ++i) {
		if (Col + i <0 || Col + i >SECTOR_NUM) continue;

		for (int j = -1; j < 1; ++j) {
			if (Row + j <0 || Row + j >SECTOR_NUM) continue;
			
			sector[Col + i][Row + j].SetObjectList(objs);

			for (int clientId : objs) {
				if (ST_INGAME != objects[clientId]->_state) continue;
				if (clientId >= MAX_USER) continue;
				if (false == CanSee(objects[id]->_id, objects[clientId]->_id)) continue;
				old_vl.insert(objects[clientId]->_id);
			}
		}
	}
	
	//TODO. �̵�->��ֹ�ã�� ������? 
	static_cast<NPC*>(objects[id])->DoRandomMove();

	//���Ͱ˻����غ���
	Col = objects[id]->_sectorCol;
	Row = objects[id]->_sectorRow;

	unordered_set<int> new_viewlist;
	for (int i = -1; i < 1; ++i) {
		if (Col + i <0 ||Col + i >SECTOR_NUM) continue;

		for (int j = -1; j < 1; ++j) {
			if (Row + j <0 || Row + j >SECTOR_NUM) continue;
			sector[Col + i][Row + j].SetObjectList(objs);

			for (int clientId : objs) {
				if (objects[clientId]->_state != ST_INGAME) continue;
				if (objects[clientId]->_id == id) continue;
				if (false == CanSee(id, objects[clientId]->_id)) continue;
				new_viewlist.insert(objects[clientId]->_id);
			}
		}
	}

	//��ο��� �˷����Ѵ�.

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
void SessionManager::AttackSession(int id, char dir)
{
	objects[id]->_vl.lock();
	unordered_set<int> vlist = objects[id]->_viewList;
	objects[id]->_vl.unlock();

	for (int npcId : vlist)	{
		std::cout << npcId << std::endl;
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

			std::cout << "���ݹ����� ���� ���� id - " << npcId << std::endl;
			static_cast<NPC*>(objects[npcId])->_isMove = false;

			objects[id]->OnAttackSuccess(objects[npcId]->_visual);
			if (objects[npcId]->OnAttackReceived(objects[id]->_damage)) {
				//������ true��ȯ�Ѵ�.->�ϴ� �׽�Ʈ�� 10���� ��Ȱ�� ������
				TimerEvent* dieev = new TimerEvent{ npcId, std::chrono::system_clock::now() + 10s, EV_NPC_DIE, 0 };
				server->InputTimerEvent(dieev);

			}

			//3�ʵڿ� �ٽ� �����̰� ����.
			if (objects[npcId]->_visual == PEACE_FIXED || objects[npcId]->_visual == PEACE_ROAMING) continue;

			//TODO. �̵�Ÿ���� peace�� �ƴѰ�쿡�� �ֵ��� �����ؾ��Ѵ�.
			TimerEvent* ev = new TimerEvent{ npcId,  std::chrono::system_clock::now() + 1s ,EV_ACTIVE_MOVE,0 };
			server->InputTimerEvent(ev);
		}
		break;
	case UP:
	case DOWN:
		for (int npcId : vlist) {
			if (abs(static_cast<int>(objects[npcId]->_x - objects[id]->_x)) > ATTACK_RANGE) {
				continue;
			}
			std::cout << "���ݹ����� ���� ���� id - " << npcId << std::endl;
			static_cast<NPC*>(objects[npcId])->_isMove = false;
			objects[id]->OnAttackSuccess(objects[npcId]->_visual);
			objects[npcId]->OnAttackReceived(objects[id]->_damage);

			TimerEvent* ev = new TimerEvent{ npcId,  std::chrono::system_clock::now() + 1s ,EV_ACTIVE_MOVE,0 };
			server->InputTimerEvent(ev);

		}
		break;
	case ALL:
		for(int npcId : vlist){
			if (abs(static_cast<int>(objects[npcId]->_x- objects[id]->_x)) > ATTACK_RANGE) {
				continue;
			}
			if (abs(static_cast<int>(objects[npcId]->_y- objects[id]->_y)) > ATTACK_RANGE) {
				continue;
			}
			std::cout << "���ݹ����� ���� ���� id - " << npcId << std::endl;
			static_cast<NPC*>(objects[npcId])->_isMove = false;
			objects[id]->OnAttackSuccess(objects[npcId]->_visual);
			objects[npcId]->OnAttackReceived(objects[id]->_damage);

			TimerEvent* ev = new TimerEvent{ npcId,  std::chrono::system_clock::now() + 1s ,EV_ACTIVE_MOVE,0 };
			server->InputTimerEvent(ev);

		}

		break;
	default:
		break;
	}
}



