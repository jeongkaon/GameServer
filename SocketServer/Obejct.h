#pragma once
class Object
{
public:
	int			_id;
	char		_name[NAME_SIZE];
	SessionState _state;
	std::mutex	_sLock;


	int _sectorCol;
	int _sectorRow;

	std::mutex	_vl;
	std::unordered_set<int> _viewList;

	int		last_move_time;

	//게임컨텐츠
	short _x, _y;
	int _visual;
	int _hp;
	int _maxHp;
	int _damage;

	char _dir;

public:
	Object();

	virtual void SendLoginPacket(){}
	virtual void SendAddPlayerPacket(int , char* ,int,int , int){}
	virtual void SendMovePacket(char dir){}
	virtual void SendMovePacket(int, int ,int ,int, char) {}

	virtual void SendRemovePlayerPacket(int){}


	//공격관련
	virtual void OnAttackSuccess(int type){}
	virtual void OnAttackReceived(int damage){}
};

