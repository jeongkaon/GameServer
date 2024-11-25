#pragma once
using namespace std;

#include "ReadWriteLock.h"

class Sector
{
	ReadWriteLock _sectorLock;
	unordered_set<int> _objectIdList;

public:
	void SetObjectList(unordered_set<int>& obj);
	void InsertObjectInSector(int id);
	void EraseObjectInSector(int id);

	unordered_set<int>& GetObjectList() { return _objectIdList; }


};

