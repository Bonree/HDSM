#pragma once
#include <string>
#include "HDSMOperateType_c.h"
using namespace std;

class Task
{
public:
	Task(OPRERATE_TYPE op, HUINT32 sn, string k, string v = "", HINT32 lExpireMinutes = -1);
	Task(OPRERATE_TYPE op, HUINT32 sn, HUINT32 limit = 100);
	~Task(void);
public:
	string buffer();
	HUINT32 get_SN();
private:
	string		m_strKey;
	string		m_strValue;
	HUINT32		m_ulOperate;
	HUINT32		m_ulSN;
	HINT32		m_lExpireMinutes;
	string		m_strPassword;
	HUINT32		m_ulShowKeysLimit;
};

