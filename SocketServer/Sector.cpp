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
	obj = _objectIdList;
	_sectorLock.unlock();

}

void Sector::EraseObjectInSector(int id)
{
	_sectorLock.lock();
	_objectIdList.erase(id);
	_sectorLock.unlock();
}


