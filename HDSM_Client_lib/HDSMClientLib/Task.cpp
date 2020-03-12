#include "Task.h"
#include "Utils.h"

Task::Task(OPRERATE_TYPE op, HUINT32 sn, string k, string v, HINT32 lExpireMinutes)
{
	m_ulOperate = op;
	if (op == OP_SHUTDOWN)
		m_strPassword = k;
	else
		m_strKey = k;

	m_strValue = v;
	m_ulSN = sn;

	if (op == OP_PUT && lExpireMinutes == 0)
		m_lExpireMinutes = -1;
	else
		m_lExpireMinutes = lExpireMinutes;

}

Task::Task(OPRERATE_TYPE op, HUINT32 sn, HUINT32 limit /* = 100 */)
{
	m_ulOperate = op;
	m_ulSN = sn;
	m_ulShowKeysLimit = limit;
}

Task::~Task(void)
{
}

string Task::buffer()
{
	//checksum(1)+sn(4)+op(4)+[keylen(4)+key]+[valuelen(4)+value]+expireminutes(4)
	string _str;
	_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
	_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));

	HUINT32 pwdLen = m_strPassword.size();
	HUINT32 keyLen = m_strKey.size();
	HUINT32 valueLen = m_strValue.size();
	HUINT32 dataLen = 0;

	switch (m_ulOperate)
	{
	case OP_ECHO:
	case OP_LENGTH:
	case OP_QUIT:
		break;
	case OP_GET:	
	case OP_ERASE:
	case OP_EXISTS:
		_str.append((const HCHAR *)&keyLen, sizeof(HUINT32));
		_str += m_strKey;
		break;
	case OP_SHUTDOWN:
		_str.append((const HCHAR *)&pwdLen, sizeof(HUINT32));
		_str += m_strPassword;
		break;
	case OP_KEYS:
		_str.append((const HCHAR *)&m_ulShowKeysLimit, sizeof(HUINT32));
		break;
	case OP_PUT:
	case OP_UPDATE:
		_str.append((const HCHAR *)&keyLen, sizeof(HUINT32));
		_str += m_strKey;
		_str.append((const HCHAR *)&valueLen, sizeof(HUINT32));
		_str += m_strValue;
		_str.append((const HCHAR *)&m_lExpireMinutes, sizeof(HINT32));
		break;
	}

	HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());

	string str;
	dataLen = sizeof(HUINT8) + _str.size();
	str.append((const HCHAR *)&dataLen, sizeof(HUINT32));
	str += checksum;
	str += _str;

	return str;
}

HUINT32 Task::get_SN()
{
	return m_ulSN;
}