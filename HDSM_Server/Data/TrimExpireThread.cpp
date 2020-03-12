#include "TrimExpireThread.h"
#include "Utils.h"
#include "../ConfigureMgr.h"
#include "../Global.h"
#include "../Logger.h"

TrimExpireThread::TrimExpireThread(IHDSMEventHandler *pHandler)
{
	m_pHandler = pHandler;
}

TrimExpireThread::~TrimExpireThread(void)
{
}

void TrimExpireThread::run()
{
	HUINT32 lTrimExpiredKeysPeriod = ConfigureMgr::get_trim_expired_keys_period();
	if (lTrimExpiredKeysPeriod > Global::SECONDS_IN_ONE_MINUTE)
		lTrimExpiredKeysPeriod = Global::SECONDS_IN_ONE_MINUTE;
	if (lTrimExpiredKeysPeriod < 30)
		lTrimExpiredKeysPeriod = 30;

	HINT32 nTicks = 0;
	while (!m_bStopped && m_pHandler != NULL)
	{
		nTicks++;
		if (nTicks % lTrimExpiredKeysPeriod == 0)
		{
			Logger::log_i("Clear expired keys[Period %d s]...", lTrimExpiredKeysPeriod);
			Logger::log_i("Clear expired keys Completed! Total: %d keys!", m_pHandler->expire());
		}

		if (nTicks % Global::SECONDS_IN_ONE_HOURS == 0)
		{
			HUINT32 uiHour = ConfigureMgr::get_hour_for_load_expired_keys_cache();
			if (uiHour > 23) 
				uiHour = 3;
			if (Utils::get_current_hour() == uiHour)
			{
				Logger::log_i("Scanning keys that expire within 24 Hours...");
				m_pHandler->load_expire_cache();
				Logger::log_i("Scanning keys that expire within 24 Hours completed!");
			}
		}
		Utils::sleep_for_seconds(1);
	}
	return;
}

