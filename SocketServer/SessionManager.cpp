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

	//npc이니셜라이즈하기
	for (int i = 0; i < MAX_USER; ++i) {
		objects[i] = new Session();
	}
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		objects[i] = new NPC();
	}

}
void SessionManager::Init()
{

	//NPC init하는부분
	ifstream in{ "npc.txt",ios::binary };

	int temp;

	int i = 0;
	int j = 0;



///temp에 -가 들어가는디??? 이상한디??
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

	std::cout << "NPC 초기화 테스트중...\n";

}

int SessionManager::AcceptClient(SOCKET& socket)
{
	//player일때 해줘야함
	
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
		//array니까 빈자리 찾아서 넣어주는거임.
		if (objects[i]->_state == ST_FREE)
			return i;
	}
	return -1;
}

void SessionManager::SetUserGameData(int id, GameData& data)
{
	//TODO.로그인할때 이함수 고쳐야함!
	if (id > MAX_USER) return;
	
	strcpy_s(objects[id]->_name, data.user_name);

}

//로그인요청 들어온거 체크해야한다.
int SessionManager::CheckLoginSession(int id)
{
	//error종류를 리턴해야한다.

	return LOGIN_OK;
}

//로긴인포 보내기 위한 함수임
//->밑에 두함수도 합쳐야함->데이터베이스에서 불러온거 기반으로 세팅하는거로
void SessionManager::LoginSession(int id, char* name)
{
	//DB에서 읽어온 구조체를 인자로 받아서 데이터 세팅해야할듯

	strcpy_s(objects[id]->_name, name);
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	objects[id]->_x = 5;//rand() % W_WIDTH;
	objects[id]->_y = 5;//rand() % W_HEIGHT;

	//근데 이렇게 하는게 좋은건지 함수포인터를 쓰는게 좋은건지 고민해봐야한다.
	objects[id]->SendLoginPacket();

	objects[id]->_sectorCol = objects[id]->_x / SECTOR_SIZE;
	objects[id]->_sectorRow = objects[id]->_y / SECTOR_SIZE;

	//비주얼도 추가해야함
	objects[id]->_visual = 1;

	//섹터에 본인을 추가해야한다.
	int col = objects[id]->_sectorCol;
	int row = objects[id]->_sectorRow;

	sector[col][row].InsertObjectInSector(id);

	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	//섹터에 본인섹터 포함 3x3에 알려줘야한다.
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

				//플레이어일때
				if (clientId < MAX_USER) {
					objects[clientId]->SendAddPlayerPacket(
						id, objects[id]->_name, objects[id]->_x, objects[id]->_y, objects[id]->_visual);
				}
				else {
					//npc일때
					//Wakeup해줘야함
					server->WakeupNpc(clientId, id);

				}
				objects[id]->SendAddPlayerPacket(clientId, objects[clientId]->_name, 
					objects[clientId]->_x, objects[clientId]->_y, objects[clientId]->_visual);


	
			}
		}
	}

}
void SessionManager::LoginSession(int id, int visual)
{
	//TODO. name 변경해야함
	strcpy_s(objects[id]->_name, "name");
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	objects[id]->_x = 5;//rand() % W_WIDTH;
	objects[id]->_y = 5;//rand() % W_HEIGHT;
	objects[id]->_visual = visual;

	//근데 이렇게 하는게 좋은건지 함수포인터를 쓰는게 좋은건지 고민해봐야한다.
	objects[id]->SendLoginPacket();

	objects[id]->_sectorCol = objects[id]->_x / SECTOR_SIZE;
	objects[id]->_sectorRow = objects[id]->_y / SECTOR_SIZE;

	//섹터에 본인을 추가해야한다.
	int col = objects[id]->_sectorCol;
	int row = objects[id]->_sectorRow;

	sector[col][row].InsertObjectInSector(id);


	//그리고 상태를 바꿔야함.
	{
		lock_guard<mutex> ll{ objects[id]->_sLock };
		objects[id]->_state = ST_INGAME;
	}

	//섹터에 본인섹터 포함 3x3에 알려줘야한다.
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
				//플레이어일때
				if (clientId < MAX_USER) {

					objects[clientId]->SendAddPlayerPacket(
						id, objects[id]->_name, objects[id]->_x, objects[id]->_y, objects[id]->_visual);

				}
				else {
					//npc일때
					//wakeup해줘야함
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

	unordered_set<int> new_viewlist;
	unordered_set<int> objs;
	for (int i = -1; i < 2; ++i) {
		if (curCol + i <0 || curCol + i >SECTOR_NUM) continue;

		for (int j = -1; j < 2; ++j) {

			int temp1 = curCol + i;
			int temp2 = curRow + j;


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
		//플레이어면
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
			//깨운다
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
	
	//TODO. 이동->장애물찾기 어케함? ->방향도 추가해주자
	static_cast<NPC*>(objects[id])->DoRandomMove();

	//섹터검색을해보자
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

	//모두에게 알려야한다.

	for (auto pl : new_viewlist) {
		if (0 == old_vl.count(pl)) {
			objects[pl]->SendAddPlayerPacket(objects[id]->_id,objects[id]->_name, 
				objects[id]->_x, objects[id]->_y, objects[id]->_visual);
		}
		else {

			//맨마지막 dir 어케할지 정해야함->몬스터도 이동하면 방향있으니까 해줘?
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
	//방향에 따라 범위를 검사해야한다->범위지정은 어케할지도 결정해야함.
	//세션매니져에 dir보내면 세션매니져에서 dir에 맞게 세션함수 부르기?
	//그럼 세션에서는 뷰리스트 참고해서 범위안에 있는 오브젝트id있으니까 걔네 공격하기   
 
	//세션매니저에서 할지? 아니면 세션에 따로 만들어놀지?

	//뷰리스트를 복사해오자
	objects[id]->_vl.lock();
	unordered_set<int> vlist = objects[id]->_viewList;
	objects[id]->_vl.unlock();

	//뷰리스트에 존재하는 id가 vlis에 넘어온다.
	//vlis에 있는애들만 검사하자.
	//


	//여기서 처리해줘야한다
	for (int npcId : vlist)	{
		std::cout << npcId << std::endl;

	}
	std::cout<<std::endl;

	switch (dir)
	{
	case LEFT:
		break;
	case RIGHT:
		break;
	case UP:
		break;
	case DOWN:
		break;
	case ALL:
		//사방 a키 눌렀을때임
		for(int npcId : vlist){
			if (abs(static_cast<int>(objects[npcId]->_x- objects[id]->_x)) > ATTACK_RANGE) {
				continue;
			}
			if (abs(static_cast<int>(objects[npcId]->_y- objects[id]->_y)) > ATTACK_RANGE) {
				continue;
			}

			std::cout << "공격범위에 들어온 몬스터 id - " << npcId << std::endl;

		}

		break;
	default:
		break;
	}
}



