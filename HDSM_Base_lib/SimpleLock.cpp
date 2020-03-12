#include "SimpleLock.h"

SimpleLock::SimpleLock(void)
{
#ifdef WIN32
	InitializeCriticalSection(&m_cs);
#else
	pthread_mutex_init(&m_mutex, NULL);
#endif
}

SimpleLock::~SimpleLock(void)
{
#ifdef WIN32
	DeleteCriticalSection(&m_cs);
#else
	pthread_mutex_destroy(&m_mutex);
#endif
}

void SimpleLock::lock()
{
#ifdef WIN32
	EnterCriticalSection(&m_cs);
#else
	pthread_mutex_lock(&m_mutex);
#endif
}

void SimpleLock::unlock()
{
#ifdef WIN32
	LeaveCriticalSection(&m_cs);
#else
	pthread_mutex_unlock(&m_mutex);
#endif
}