#pragma once
#include "Result.h"
#include "Task.h"
#include "BaseSocket.h"
#include "TypeDefine.h"

class HDSMClientImpl
{
public:
	HDSMClientImpl(void);
	~HDSMClientImpl(void);
public:
	Result echo();
	HBOOL do_task(Task &task, Result &result);
	HBOOL initialize();
	void unintialize();
public:
	HUINT32 get_sock();
	HBOOL is_valid();
public:
	Result get(const string &k);
	Result put(const string &k, const string &v, HINT32 lExpireMinutes);
	Result update(const string &k, const string &v, HINT32 lExpireMinutes);
	Result erase(const string &k);
	Result exists(const string &k);
	Result length();
	Result quit();
	Result shutdown(const string &password);
	Result keys(HUINT32 limit);
public:
	BaseSocket	m_sock;
	string		m_strSrvIP;
	HUINT16		m_usSrvPort;
	HINT32		m_iTimeout;
	HUINT32		m_ulMaxKeyLen;
	HUINT32		m_ulMaxValueLen;
	HBOOL		m_bValid;
	HUINT32		m_ulCurSN;
};

