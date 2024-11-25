#pragma once
#include "ReadWriteLock.h"

class Object
{
public:
	int			_id;
	char		_name[NAME_SIZE];
	SessionState _state;
	std::mutex	_sLock;

	int _sectorCol;
	int _sectorRow;

	ReadWriteLock	_vl;
	std::unordered_set<int> _viewList;

	int		last_move_time;

	//°ÔÀÓÄÁÅÙÃ÷
	short _x, _y;
	int _visual;
	int _hp; 
	int _maxHp;
	int _damage;

	char _dir;

public:
	Object();

	virtual void init(void* ){}
	virtual void SendLoginPacket(){}
	virtual void SendAddPlayerPacket(int , char* ,int,int , int){}
	virtual void SendMovePacket(char dir){}
	virtual void SendMovePacket(int, int ,int ,int, char) {}
	virtual void SendRemovePlayerPacket(int){}


	//°ø°Ý°ü·Ã
	virtual void OnAttackSuccess(int type,int,int){}
	virtual bool OnAttackReceived(int damage, int dir) { return false; }
};

