#include "SimpleTimer.h"
#include "Utils.h"

SimpleTimer::SimpleTimer(ISimpleTimerHandler *pHandler, HUINT32 uiElaspeMs)
{
	m_pHandler = pHandler;
	m_uiElaspeMs = uiElaspeMs;
}

SimpleTimer::~SimpleTimer(void)
{
}

void SimpleTimer::run()
{
	HUINT32 ulCurTicks = 0;
	while (!m_bStopped && m_pHandler != NULL)
	{
		Utils::sleep_for_milliseconds(m_uiElaspeMs);
		ulCurTicks++;
		m_pHandler->on_timer(ulCurTicks);
	}
	return;
}

