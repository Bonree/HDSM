#pragma once
#include <string>
#include <vector>
#include "HDSMOperateType_c.h"
#include "TypeDefine.h"
using namespace std;

class ResultImpl
{
public:
	ResultImpl(void);
	~ResultImpl(void);
public:
	HUINT32	m_bRet;
	string	m_strKey;
	string	m_strValue;
	HUINT32	m_ulLength;
	HUINT32	m_ulOperate;
	HUINT32	m_ulSN;
	HINT32	m_lExpireMinutes;
	string	m_strErrInfo;
public:
	HUINT32	m_ulProcessTime;
	HUINT32	m_ulTotalTime;
public:
	HBOOL	m_bValid;
public:
	HUINT32	m_ulMaxKeyLength;
	HUINT32	m_ulMaxValueLength;
public:
	vector<KeyValueInfo> m_vecKeysInfo;
};

