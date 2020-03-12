#include "HDSMInitThread.h"

HDSMInitThread::HDSMInitThread(IHDSMEventHandler *pHandler, HUINT32 nStartIndex, HUINT32 nEndIndex)
{
	m_pHandler = pHandler;
	m_nStartIndex = nStartIndex;
	m_nEndIndex = nEndIndex;
}


HDSMInitThread::~HDSMInitThread(void)
{
}


void HDSMInitThread::run()
{
	if (m_pHandler != NULL)
		m_pHandler->init_some_hdlists(m_nStartIndex, m_nEndIndex);
	return;
}