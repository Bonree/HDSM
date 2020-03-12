#include "OplogWriteThread.h"


OplogWriteThread::OplogWriteThread(IOplogManager *pManager)
{
	m_pManager = pManager;
}


OplogWriteThread::~OplogWriteThread(void)
{
}

void OplogWriteThread::run()
{
	if (m_pManager != NULL)
	{
		while (!m_bStopped)
			m_pManager->enter_write_loop();
	}
	return;
}