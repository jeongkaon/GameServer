#pragma once
#include <bitset>
class MapManager
{
	
	//bit�� ����ϴ°ŷ� �ٲ��� 2000 2000 �̴ϱ� 
	//�ϴ� bool�� �ϰ� bit�� �ٲٴ¹�� �����ϱ�
	bool _moveInfo[150][150]{};

public:
	MapManager();
	void Init();
	void InitNpc();

	bool IsCanGoCheck(int dir, int& x, int& y);

	//�ʸŴ������� npc��ġ��, �浹�̷��� �� üũ�ϴ°ŷ� ����.��

	

};

