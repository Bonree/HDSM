#pragma once
#ifdef WIN32
#include "SimpleLock.h"
#else
#include <pthread.h>
#endif

#include "TypeDefine.h"

class RWLock
{
public:
	RWLock(void);
	~RWLock(void);
public:
	HBOOL lock(HINT32 nLevel);
	void unlock();
public:
	enum LOCK_LEVEL
	{
		LOCK_LEVEL_NONE = 0,
		LOCK_LEVEL_READ,
		LOCK_LEVEL_WRITE
	};
private:
#ifdef WIN32
	HINT32		m_iCurLevel;
	SimpleLock	m_lock;
	HINT32		m_iReadCount;   
	HANDLE		m_hUnlockEvent; 
	HANDLE		m_hAccessMutex;
#else
	pthread_rwlock_t m_rwLock;
#endif
};

