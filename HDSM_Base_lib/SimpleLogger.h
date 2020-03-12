#pragma once
#include <string>
#include <fstream>
#include "SimpleLock.h"
#include "TypeDefine.h"
using namespace std;

#ifndef WIN32
#define MAX_PATH 260
#endif

class SimpleLogger
{
public:
	SimpleLogger(const string &strLogFilePath, HUINT32 nlevel, HUINT32 ulMaxFileSize);
	~SimpleLogger(void);
public:
	enum LOG_LEVEL
	{
		LEVEL_DEBUG = 0,
		LEVEL_INFO,
		LEVEL_WARNNING,
		LEVEL_ERROR,
	};
public:
	HBOOL	log(HUINT32 nlevel, const HCHAR *pszlevelstr,  const HCHAR *pInfo);
private:
	HBOOL	check_file_size();
private:
	ofstream	m_ofs;
	SimpleLock	m_lock;
	string		m_strLogFilePath;
	HUINT32		m_ulMaxFileSize;
	HUINT32		m_nLevel;
};
