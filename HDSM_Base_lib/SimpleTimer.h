#pragma once
#include "BaseThread.h"
#include "ISimpleTimerHandler.h"
#include "TypeDefine.h"

class SimpleTimer :
	public BaseThread
{
public:
	SimpleTimer(ISimpleTimerHandler *pHandler, HUINT32 uiElaspeMs);
	~SimpleTimer(void);
private:
	virtual void run();
private:
	ISimpleTimerHandler *m_pHandler;
	HUINT32		m_uiElaspeMs;
};

