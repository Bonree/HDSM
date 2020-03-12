#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif
#include "SimpleNotifier.h"

SimpleNotifier::SimpleNotifier(void)
{
#ifdef WIN32
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_cond, NULL);
	m_state = false;
#endif
}

SimpleNotifier::~SimpleNotifier(void)
{
#ifdef WIN32
	if (m_hEvent != NULL) 
		CloseHandle(m_hEvent);
#else
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_mutex);
#endif
}

void SimpleNotifier::awake()
{
#ifdef WIN32
	if (m_hEvent != NULL)
		SetEvent(m_hEvent);
#else
	SetEventImpl();
#endif
	return;
}

void SimpleNotifier::wait()
{
#ifdef WIN32
	if (m_hEvent != NULL)
		WaitForSingleObject(m_hEvent, INFINITE);
#else
	WaitImpl();
#endif
	return;
}

void SimpleNotifier::rest()
{
#ifdef WIN32
	if (m_hEvent != NULL)
		ResetEvent(m_hEvent);
#else
	ResetEventImpl();
#endif
	return;
}

#ifndef WIN32
void SimpleNotifier::SetEventImpl()
{
	pthread_mutex_lock(&m_mutex);
	m_state = true;  
	pthread_cond_broadcast(&m_cond);
	pthread_mutex_unlock(&m_mutex);  
	return;
}

void SimpleNotifier::ResetEventImpl()
{
	pthread_mutex_lock(&m_mutex);
	m_state = false;  
	pthread_mutex_unlock(&m_mutex);  
	return;
}

void SimpleNotifier::WaitImpl()
{
	pthread_mutex_lock(&m_mutex);
	while (!m_state)
		pthread_cond_wait(&m_cond, &m_mutex);
	pthread_mutex_unlock(&m_mutex);
	return;
}
#endif