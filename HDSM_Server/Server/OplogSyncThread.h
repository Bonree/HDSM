#pragma once
#include "BaseThread.h"
#include "ITaskCenter.h"
#include "OplogSynchronizer.h"
#include "TypeDefine.h"


class OplogSyncThread 
	: public BaseThread
{
public:
	OplogSyncThread(ITaskCenter *pTaskCenter);
	virtual ~OplogSyncThread(void);
private:
	virtual void run();
private:
	HBOOL process_sync_data(HUINT64 ullAckIndex, const HCHAR *pszBuf, HUINT32 nBufLen);
	HBOOL process_one_op_data(const HCHAR *pszBuf, HUINT32 nBufLen);
private:
	ITaskCenter	*m_pTaskCenter;
	OplogSynchronizer *m_pSynchronizer;
	HUINT32		m_uiSyncPeriod;
	HUINT32		m_ulToatlOps;
};

