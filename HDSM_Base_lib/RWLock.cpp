#include "RWLock.h"

RWLock::RWLock(void)
{
#ifdef WIN32
	m_iCurLevel = LOCK_LEVEL_NONE;
	m_iReadCount = 0;
	m_hUnlockEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hAccessMutex = CreateMutex( NULL, FALSE, NULL);
#else
	pthread_rwlock_init(&m_rwLock, NULL);
#endif
}

RWLock::~RWLock(void)
{
#ifdef WIN32
	if (m_hAccessMutex) 
		CloseHandle(m_hAccessMutex);
	if (m_hUnlockEvent) 
		CloseHandle(m_hUnlockEvent);
#else
	pthread_rwlock_destroy(&m_rwLock);
#endif
}

HBOOL RWLock::lock(HINT32 iLevel)
{
#ifdef WIN32
	HBOOL bresult = true;
	DWORD waitResult = 0;

	waitResult = WaitForSingleObject(m_hAccessMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) 
		return false;

	if (iLevel == LOCK_LEVEL_READ && m_iCurLevel != LOCK_LEVEL_WRITE)
	{
		m_lock.lock();
		m_iCurLevel = iLevel;
		m_iReadCount += 1;
		ResetEvent(m_hUnlockEvent);
		m_lock.unlock();
	}
	else if (iLevel == LOCK_LEVEL_READ && m_iCurLevel == LOCK_LEVEL_WRITE)
	{
		waitResult = WaitForSingleObject(m_hUnlockEvent, INFINITE);
		if (waitResult == WAIT_OBJECT_0)
		{
			m_lock.lock();
			m_iCurLevel = iLevel;
			m_iReadCount += 1;
			ResetEvent(m_hUnlockEvent);
			m_lock.unlock();
		}
		else 
			bresult = false;
	}
	else if (iLevel == LOCK_LEVEL_WRITE && m_iCurLevel == LOCK_LEVEL_NONE)
	{
		m_lock.lock();
		m_iCurLevel = iLevel;
		ResetEvent(m_hUnlockEvent);
		m_lock.unlock();
	}
	else if (iLevel == LOCK_LEVEL_WRITE && m_iCurLevel != LOCK_LEVEL_NONE)
	{
		waitResult = ::WaitForSingleObject(m_hUnlockEvent, INFINITE);
		if (waitResult == WAIT_OBJECT_0)
		{
			m_lock.lock();
			m_iCurLevel = iLevel;
			ResetEvent(m_hUnlockEvent);
			m_lock.unlock();
		}
		else 
			bresult = false;
	}

	ReleaseMutex(m_hAccessMutex);
	return bresult;
#else
	if (iLevel == LOCK_LEVEL_READ)
		pthread_rwlock_rdlock(&m_rwLock);
	if (iLevel == LOCK_LEVEL_WRITE)
		pthread_rwlock_wrlock(&m_rwLock);
	return true;
#endif
}

void RWLock::unlock()
{
#ifdef WIN32
	m_lock.lock();
	if (m_iCurLevel == LOCK_LEVEL_READ)
	{
		m_iReadCount--;
		if (m_iReadCount == 0) 
		{
			m_iCurLevel = LOCK_LEVEL_NONE;
			SetEvent(m_hUnlockEvent);
		}
	}
	else if (m_iCurLevel == LOCK_LEVEL_WRITE)
	{
		m_iCurLevel = LOCK_LEVEL_NONE;
		SetEvent(m_hUnlockEvent);
	}
	m_lock.unlock();
#else
	pthread_rwlock_unlock(&m_rwLock);
#endif
	return;
}