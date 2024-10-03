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

	obj.insert(temp.begin(), temp.end()); // _objectIdList의 모든 요소를 obj에 추가합니다.

}

void Sector::EraseObjectInSector(int id)
{
	_sectorLock.lock();
	_objectIdList.erase(id);
	_sectorLock.unlock();
}


