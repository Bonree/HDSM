#include "BaseThread.h"

BaseThread::BaseThread(void)
{
#ifdef WIN32
	m_hThread = NULL;
#else
	m_thread_t = 0;
#endif
	m_bStopped = false;
}

BaseThread::~BaseThread(void)
{
	m_bStopped = true;
#ifdef WIN32
	if (m_hThread != NULL)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
#else
	m_thread_t = 0;
#endif
}

HBOOL BaseThread::start()
{
#ifdef WIN32
	m_hThread = ((HANDLE)_beginthreadex(NULL, 0, thread_func, this, 0, &m_uiThreadId));
	return (m_hThread != NULL);
#else
	pthread_create(&m_thread_t, NULL, thread_func, this);
	return (m_thread_t != 0);
#endif

}

void BaseThread::stop()
{
	m_bStopped = true;
}

#ifdef WIN32
HUINT32 __stdcall BaseThread::thread_func(void *pArg)
#else
void* BaseThread::thread_func(void* pArg)
#endif
{
	BaseThread *pThis = (BaseThread *)pArg;
	if (pThis != NULL)
		pThis->run();
	return 0;
}
