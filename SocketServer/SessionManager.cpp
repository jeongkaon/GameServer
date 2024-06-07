#include "stdafx.h"
#include "SessionManager.h"

using namespace std;
Sector SessionManager::sector[SECTOR_NUM][SECTOR_NUM]{};


void SessionManager::Init()
{
	
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

SessionManager::SessionManager()
{
	std::cout << "herel!" << std::endl;


	//npc�̴ϼȶ������ϱ�
	for (int i = 0; i < MAX_USER; ++i) {
		objects[i] = new Session();
	}
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		objects[i] = new NPC();
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

//�α��ο�û ���°� üũ�ؾ��Ѵ�.
int SessionManager::CheckLoginSession(int id)
{
	//error������ �����ؾ��Ѵ�.

	return LOGIN_OK;
}

//�α����� ������ ���� �Լ���
void SessionManager::LoginSession(int id, char* name)
{
	//DB���� �о�� ����ü�� ���ڷ� �޾Ƽ� ������ �����ؾ��ҵ�

	strcpy_s(objects[id]->_name, name);
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	objects[id]->_x = 5;//rand() % W_WIDTH;
	objects[id]->_y = 5;//rand() % W_HEIGHT;

	//�ٵ� �̷��� �ϴ°� �������� �Լ������͸� ���°� �������� ����غ����Ѵ�.
	objects[id]->SendLoginPacket();

	objects[id]->_sectorCol = objects[id]->_x / SECTOR_SIZE;
	objects[id]->_sectorRow = objects[id]->_y / SECTOR_SIZE;

	//���־� �߰��ؾ���
	objects[id]->_visual = 1;

	//���Ϳ� ������ �߰��ؾ��Ѵ�.
	int col = objects[id]->_sectorCol;
	int row = objects[id]->_sectorRow;

	sector[col][row].InsertObjectInSector(id);

	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	//���Ϳ� ���μ��� ���� 3x3�� �˷�����Ѵ�.
	unordered_set<int> objs;
	for (int i = -1; i < 1; ++i) {
		if (col + i <0 || col + i >SECTOR_NUM) continue;

		for (int j = -1; j < 1; ++j) {
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
					Wakeup�������

				}
				objects[id]->SendAddPlayerPacket(clientId, objects[clientId]->_name, 
					objects[clientId]->_x, objects[clientId]->_y, objects[clientId]->_visual);


	
			}
		}
	}

}

void SessionManager::LoginSession(int id, int visual)
{
	//TODO. name �����ؾ���
	strcpy_s(objects[id]->_name, "name");
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	objects[id]->_x = 5;//rand() % W_WIDTH;
	objects[id]->_y = 5;//rand() % W_HEIGHT;
	objects[id]->_visual = visual;

	//�ٵ� �̷��� �ϴ°� �������� �Լ������͸� ���°� �������� ����غ����Ѵ�.
	objects[id]->SendLoginPacket();

	objects[id]->_sectorCol = objects[id]->_x / SECTOR_SIZE;
	objects[id]->_sectorRow = objects[id]->_y / SECTOR_SIZE;

	//���Ϳ� ������ �߰��ؾ��Ѵ�.
	int col = objects[id]->_sectorCol;
	int row = objects[id]->_sectorRow;

	sector[col][row].InsertObjectInSector(id);


	//�׸��� ���¸� �ٲ����.
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	//���Ϳ� ���μ��� ���� 3x3�� �˷�����Ѵ�.
	unordered_set<int> objs;
	for (int i = -1; i < 1; ++i) {
		if (col + i <0 || col + i >SECTOR_NUM) continue;

		for (int j = -1; j < 1; ++j) {
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
					wakeup�������
					
				}
				objects[id]->SendAddPlayerPacket(clientId, objects[clientId]->_name,
					objects[clientId]->_x, objects[clientId]->_y, objects[clientId]->_visual);



			}
		}
	}

}


void SessionManager::MoveSession(int id, CS_MOVE_PACKET* packet)
{

	objects[id]->last_move_time = packet->move_time;

	int x = objects[id]->_x;
	int y = objects[id]->_y;

	//�̹��� ������ �ٸ��� Ȯ���ؾ��Ѵ�.
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
	for (int i = -1; i < 1; ++i) {
		if (curCol + i <0 || curCol + i >SECTOR_NUM) continue;

		for (int j = -1; j < 1; ++j) {
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

	objects[id]->SendMovePacket();
	for (int clientId : new_viewlist) {
		//�÷��̾��
		if (clientId < MAX_USER) {
			objects[clientId]->_vl.lock();
			if (objects[clientId]->_viewList.count(id)) {
				objects[clientId]->_vl.unlock();
				objects[clientId]->SendMovePacket(id, x, y, objects[id]->last_move_time);

			}
			else {
				objects[clientId]->_vl.unlock();
				objects[clientId]->SendAddPlayerPacket(id, objects[id]->_name, 
					objects[id]->_x, objects[id]->_y, objects[id]->_visual);
					
			}
		}
		else {
			�����
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
	std::unordered_set<int> old_vl;
	std::unordered_set<int> objs;

	int Col = static_cast<NPC*>(objects[id])->_sectorCol;
	int Row = static_cast<NPC*>(objects[id])->_sectorRow;

	for (int i = -1; i < 1; ++i) {
		if (Col + i <0 || Col + i >SECTOR_NUM) continue;

		for (int j = -1; j < 1; ++j) {
			if (Row + j <0 || Row + j >SECTOR_NUM) continue;

			sector[Col + i][Row + j].SetObjectList(objs);
			for (int clientId : objs) {
				if (ST_INGAME != objects[clientId]->_state) continue;

				if (clientId > MAX_USER) continue;
				if (false == CanSee(objects[id]->_id, objects[clientId]->_id)) continue;
				old_vl.insert(objects[clientId]->_id);

			}
		}
	}

	static_cast<NPC*>(objects[id])->DoRandomMove();
		
	Col = static_cast<NPC*>(objects[id])->_sectorCol;
	Row = static_cast<NPC*>(objects[id])->_sectorRow;

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
				new_viewlist.insert(objects[id]->_id);
			}
		}
	}

	//��ο��� �˷����Ѵ�.

	for (auto pl : new_viewlist) {
		if (0 == old_vl.count(pl)) {
			// �÷��̾��� �þ߿� ����
		
			objects[pl]->SendAddPlayerPacket(objects[id]->_id,objects[id]->_name, objects[id]->_x, objects[id]->_y, objects[id]->_visual);
		}
		else {
			// �÷��̾ ��� ���� ����.
			objects[pl]->SendMovePacket(objects[id]->_id, objects[id]->_x, objects[id]->_y, objects[id]->last_move_time);

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

