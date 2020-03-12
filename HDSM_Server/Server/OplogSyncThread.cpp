#include "OplogSyncThread.h"
#include "Utils.h"
#include "../ConfigureMgr.h"
#include "../Logger.h"
#include <string.h>

OplogSyncThread::OplogSyncThread(ITaskCenter *pTaskCenter)
{
	m_pTaskCenter = pTaskCenter;
	m_pSynchronizer = new OplogSynchronizer();
	m_uiSyncPeriod = ConfigureMgr::get_oplog_sync_period();
	if (m_uiSyncPeriod > 15) m_uiSyncPeriod = 15;
	if (m_uiSyncPeriod < 2) m_uiSyncPeriod = 2;
	m_ulToatlOps = 0;
}


OplogSyncThread::~OplogSyncThread(void)
{
	if (m_pSynchronizer != NULL)
		delete m_pSynchronizer;
	m_pSynchronizer = NULL;
}

//len(4)+checksum(1)+op(4)+[keylen(4)+key]+[valuelen(4)+value]+expireminutes[4]+ts[8]
HBOOL OplogSyncThread::process_sync_data(HUINT64 ullAckIndex, const HCHAR *pszBuf, HUINT32 nBufLen)
{
	HUINT32 offset = 0;
	HUINT32 count = 0;
	while (offset < nBufLen)
	{
		HUINT32 ulDataLen = 0;
		memcpy(&ulDataLen, pszBuf+offset, sizeof(HUINT32));
		offset += sizeof(HUINT32);
		if (offset + ulDataLen > nBufLen)
			break;
		if (process_one_op_data(pszBuf+offset, ulDataLen))
			count++;
		offset += ulDataLen;
	}

	Logger::log_i("Synced The No.[%I64d] successed([%d] oplogs)!Total synced [%d] oplogs!", ullAckIndex, count, m_ulToatlOps);
	return true;
}

HBOOL OplogSyncThread::process_one_op_data(const HCHAR *pszBuf, HUINT32 nBufLen)
{
	Task task(NULL, pszBuf, nBufLen);
	if (task.is_valid())
	{
		if (m_pTaskCenter != NULL)
		{
			m_ulToatlOps++;
			m_pTaskCenter->push_task(task);
			return true;
		}
	}
	return false;
}

void OplogSyncThread::run()
{
	Logger::log_i("Oplogs sync thread startup!");
	HUINT64 ullAckIndex = 0;
	while (!m_bStopped && m_pTaskCenter!= NULL && m_pSynchronizer != NULL)
	{
		string str;
		ullAckIndex = m_pSynchronizer->sync(str);
		if (str.length() > 0 && ullAckIndex > 0)
		{
			if (process_sync_data(ullAckIndex, str.c_str(), str.length()))
			{
				HBOOL ret = false;
				do
				{
					ret = m_pSynchronizer->ack(ullAckIndex);
					if (!ret)
						Utils::sleep_for_milliseconds(100);
				}
				while (!ret);
			}
		}
		else
		{
			if (str.compare("NODATA") == 0)
				Logger::log_d("No oplogs need to sync!");
			else
				Logger::log_w("synced oplogs failed!");
			Utils::sleep_for_seconds(m_uiSyncPeriod);
		}
	}
	return;
}