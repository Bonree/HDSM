#pragma once
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "TypeDefine.h"

class SimpleLock
{
public:
	SimpleLock(void);
	~SimpleLock(void);
public:
	void lock();
	void unlock();
private:
#ifdef WIN32
	CRITICAL_SECTION m_cs;
#else
	mutable pthread_mutex_t m_mutex;
#endif
};

