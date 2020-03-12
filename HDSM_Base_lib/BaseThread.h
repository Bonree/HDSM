#pragma once
#ifdef WIN32
#include <WinSock2.h>  
#include <process.h>
#else
#include <pthread.h>
#endif

#include "TypeDefine.h"

class BaseThread
{
public:
	BaseThread(void);
	virtual ~BaseThread(void);
public:
	HBOOL start();
	void stop();
protected:
	virtual void run() = 0;
private:
#ifdef WIN32
	static unsigned __stdcall thread_func(void *pArg);
	HANDLE			m_hThread;
#else
	static void* thread_func(void* arg);
	pthread_t		m_thread_t;
#endif
protected:
	HUINT32	m_uiThreadId;
	HBOOL	m_bStopped;
};

