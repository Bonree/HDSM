#pragma once
#include "SimpleLock.h"
#include "IBasePool.h"
#include <queue>
#include "TypeDefine.h"
using namespace std;

class BufferPool 
	: public IBasePool
{
public:
	BufferPool(HUINT32 nCount, HUINT32 nSize = 4096);
	virtual ~BufferPool(void);
public:
	virtual HINT32	lease();
	virtual void ret(HUINT32 iIndex);
	virtual void *get(HUINT32 iIndex);
	virtual HUINT32 size();
private:
	HUINT32		m_nSize;
	HUINT32		m_nCount;
	HCHAR		*m_pBuffer;
	queue<HUINT32> m_unused;
	SimpleLock	m_lock;
};

