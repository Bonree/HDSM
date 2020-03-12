#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <stdio.h>
#endif

#include "SimpleLogger.h"
#include "Utils.h"
#include <string.h>

SimpleLogger::SimpleLogger(const string &strLogFilePath, HUINT32 nlevel, HUINT32 ulMaxFileSize)
{
	m_ulMaxFileSize = (ulMaxFileSize < 1*1024*1024) ? (1*1024*1024) : ulMaxFileSize;
	m_nLevel = nlevel;

	if (strLogFilePath.length() == 0)
		Utils::get_current_path(m_strLogFilePath);
	else
		m_strLogFilePath = strLogFilePath;

	Utils::create_dirs(m_strLogFilePath);
	m_strLogFilePath += "/hdsm.log";

	m_ofs.open(m_strLogFilePath, ios::out|ios::app);
}

SimpleLogger::~SimpleLogger(void)
{
	if (m_ofs.is_open()) 
		m_ofs.close();
}

HBOOL SimpleLogger::check_file_size()
{
	if (m_ofs.is_open())
	{
		HINT64 len = m_ofs.tellp();
		if (len > m_ulMaxFileSize)
		{
			m_ofs.close();
			Utils::delete_file(m_strLogFilePath +".1");
			Utils::rename_file(m_strLogFilePath, m_strLogFilePath +".1");
			m_ofs.open(m_strLogFilePath, ios::out|ios::app);
			return m_ofs.is_open();
		}
	}
	else
	{
		m_ofs.open(m_strLogFilePath, ios::out|ios::app);
		return m_ofs.is_open();
	}

	return true;
}

HBOOL SimpleLogger::log(HUINT32 nlevel, const HCHAR *pszlevelstr,  const HCHAR *pInfo)
{
	string str = Utils::get_fmt_local_time() + " [" + pszlevelstr + "] " + pInfo + "\n";
	if (nlevel >= m_nLevel)
		printf("%s", str.c_str());

	m_lock.lock();
	if (check_file_size())
	{
		m_ofs << str;
		m_ofs.flush();
	}
	m_lock.unlock();

	return true;
}

