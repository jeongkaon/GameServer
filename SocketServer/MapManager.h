#pragma once
#include <bitset>
class MapManager
{
	
	//bit�� ����ϴ°ŷ� �ٲ��� 2000 2000 �̴ϱ� 
	//�ϴ� bool�� �ϰ� bit�� �ٲٴ¹�� �����ϱ�
	std::array<std::array<bool,150>,150> _moveInfo;


public:
	MapManager();
	void InitMapInfo();
//	void InitNpcInfo();

	bool IsCanGoCheck(int dir, int& x, int& y);

	//�ʸŴ������� npc��ġ��, �浹�̷��� �� üũ�ϴ°ŷ� ����.��

	

};

