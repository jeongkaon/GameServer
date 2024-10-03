#include "stdafx.h"
#include "Sector.h"

void Sector::InsertObjectInSector(int id)
{
	_sectorLock.lock();
	_objectIdList.insert(id);
	_sectorLock.unlock();
}

void Sector::SetObjectList(unordered_set<int>& obj)
{
	if (_objectIdList.size() == 0) return;
	
	_sectorLock.lock();
	unordered_set<int> temp = _objectIdList;
	_sectorLock.unlock();

	obj.insert(temp.begin(), temp.end()); // _objectIdList�� ��� ��Ҹ� obj�� �߰��մϴ�.

}

void Sector::EraseObjectInSector(int id)
{
	_sectorLock.lock();
	_objectIdList.erase(id);
	_sectorLock.unlock();
}


