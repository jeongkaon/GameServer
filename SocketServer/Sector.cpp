#include "stdafx.h"
#include "Sector.h"
void Sector::InsertObjectInSector(int id)
{
	_sectorLock.WriteLock();
	_objectIdList.insert(id);
	_sectorLock.WriteUnlock();
}

void Sector::SetObjectList(unordered_set<int>& obj)
{
	if (_objectIdList.size() == 0) return;
	_sectorLock.WriteLock();
	obj = _objectIdList;
	_sectorLock.WriteUnlock();


}

void Sector::EraseObjectInSector(int id)
{
	_sectorLock.WriteLock();
	_objectIdList.erase(id);
	_sectorLock.WriteUnlock();

}


