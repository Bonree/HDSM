#pragma once
#ifdef WIN32
#else
#include <pthread.h>
#endif

#include "TypeDefine.h"

class SimpleNotifier
{
public:
	SimpleNotifier(void);
	~SimpleNotifier(void);
public:
	void awake();
	void wait();
	void rest();
private:
#ifdef WIN32
	HANDLE m_hEvent; 
#else
	volatile HBOOL   m_state;  
	pthread_mutex_t m_mutex;  
	pthread_cond_t  m_cond;
private:
	void SetEventImpl();
	void ResetEventImpl();
	void WaitImpl();
#endif
};

