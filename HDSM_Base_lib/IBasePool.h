#pragma once
#include "TypeDefine.h"

struct IBasePool
{
	//return count of unused nodes
	virtual HUINT32 size() = 0;

	//return the Node which leased
	virtual void ret(HUINT32 index) = 0;

	//lease a unused Node in the Pool
	virtual HINT32 lease() = 0;

	//get a Node by it's Index
	virtual void *get(HUINT32 index) = 0;
};