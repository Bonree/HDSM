#include "HDLinkedListNode.h"
#include "Utils.h"
#include "../Global.h"
#include "../Logger.h"
#include <string.h>

HDLinkedListNode::HDLinkedListNode()
{
	m_bValid = false;
}

HDLinkedListNode::HDLinkedListNode(const HDLinkedListNode &node)
{
	m_ucFlag = node.m_ucFlag;
	m_llNextPtr = node.m_llNextPtr;
	m_llPrePtr = node.m_llPrePtr;
	m_llSelfPtr = node.m_llSelfPtr;
	m_strKey = node.m_strKey;
	m_strValue = node.m_strValue;
	m_bValid = node.m_bValid;
	m_ullTimestamp = node.m_ullTimestamp;
	m_lExpireMinutes = node.m_lExpireMinutes;
}

HDLinkedListNode::HDLinkedListNode(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	m_ucFlag = 0x00;
	m_llNextPtr = 0;
	m_llPrePtr = 0;
	m_llSelfPtr = 0;
	m_ullTimestamp = ts;
	m_bValid = true;
	m_lExpireMinutes = lExpireMinutes;
	if (k.length() <= Global::MAX_KEY_LENGTH)
		m_strKey = k;
	else
		m_strKey = k.substr(0, Global::MAX_KEY_LENGTH);

	if(v.length() <= Global::MAX_VALUE_LENGTH)
		m_strValue = v;
	else
		m_strValue = k.substr(0, Global::MAX_VALUE_LENGTH);
}

HDLinkedListNode::HDLinkedListNode(const HCHAR *pszBuf, HUINT32 nBufLen)
{
	m_bValid = false;
	if (pszBuf != NULL && nBufLen > 0)
	{
		HUINT8 checksum = pszBuf[0];
		if (checksum == Utils::get_check_sum(pszBuf+sizeof(checksum), nBufLen-sizeof(checksum)))
		{
			HUINT32 offset = sizeof(checksum);
			memcpy(&m_ucFlag, pszBuf+offset, sizeof(m_ucFlag));
			offset += sizeof(m_ucFlag);

			memcpy(&m_ullTimestamp, pszBuf+offset, sizeof(m_ullTimestamp));
			offset += sizeof(m_ullTimestamp);

			memcpy(&m_lExpireMinutes, pszBuf+offset, sizeof(m_lExpireMinutes));
			offset += sizeof(m_lExpireMinutes);

			memcpy(&m_llNextPtr, pszBuf+offset, sizeof(m_llNextPtr));
			offset += sizeof(m_llNextPtr);

			memcpy(&m_llPrePtr, pszBuf+offset, sizeof(m_llPrePtr));
			offset += sizeof(m_llPrePtr);

			memcpy(&m_llSelfPtr, pszBuf+offset, sizeof(m_llSelfPtr));
			offset += sizeof(m_llSelfPtr);

			HUINT32 ulKeyLen = 0;
			memcpy(&ulKeyLen, pszBuf+offset, sizeof(ulKeyLen));
			offset += sizeof(ulKeyLen);

			HUINT32 ulValueLen = 0;
			memcpy(&ulValueLen, pszBuf+offset, sizeof(ulValueLen));
			offset += sizeof(ulValueLen);

			m_strKey.append(pszBuf+offset, ulKeyLen);
			offset += ulKeyLen;

			m_strValue.append(pszBuf+offset, ulValueLen);
			offset += ulValueLen;

			m_bValid = true;
		}
		else
		{
			Logger::log_w("Checksum is Invalid!");
		}
	}
}

HDLinkedListNode::~HDLinkedListNode(void)
{
	m_bValid = false;
	m_strKey.clear();
	m_strValue.clear();
}

const string &HDLinkedListNode::key()
{
	return m_strKey;
}

const string &HDLinkedListNode::value()
{
	return m_strValue;
}

void HDLinkedListNode::set_deleted(HBOOL deleted)
{
	if (deleted) 
		m_ucFlag |= 0x01;
	else
		m_ucFlag &= 0x00;
}

HUINT64 HDLinkedListNode::get_time_stamp() const
{
	return m_ullTimestamp;
}

HBOOL HDLinkedListNode::is_deleted()
{
	return (m_ucFlag&0x01);
}

string HDLinkedListNode::buffer()
{
	string _str;
	_str.append((const HCHAR *)&m_ucFlag, sizeof(m_ucFlag));
	_str.append((const HCHAR *)&m_ullTimestamp, sizeof(m_ullTimestamp));
	_str.append((const HCHAR *)&m_lExpireMinutes, sizeof(m_lExpireMinutes));
	_str.append((const HCHAR *)&m_llNextPtr, sizeof(m_llNextPtr));
	_str.append((const HCHAR *)&m_llPrePtr, sizeof(m_llPrePtr));
	_str.append((const HCHAR *)&m_llSelfPtr, sizeof(m_llSelfPtr));

	HUINT32 ulKeyLen = m_strKey.length();
	_str.append((const HCHAR *)&ulKeyLen, sizeof(ulKeyLen));

	HUINT32 ulValueLen = m_strValue.length();
	_str.append((const HCHAR *)&ulValueLen, sizeof(ulValueLen));

	if (m_strKey.length() <= Global::MAX_KEY_LENGTH)
		_str += m_strKey;
	else
		_str += m_strKey.substr(0, Global::MAX_KEY_LENGTH);
	
	if (m_strValue.length() <= Global::MAX_VALUE_LENGTH)
		_str += m_strValue;
	else
		_str += m_strValue.substr(0, Global::MAX_VALUE_LENGTH);

	HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());

	string str;
	str += checksum;
	str += _str;

	return str;
}

HBOOL HDLinkedListNode::is_valid()
{
	return m_bValid;
}

HINT64 HDLinkedListNode::next() const
{
	return m_llNextPtr;
}

HINT64 HDLinkedListNode::pre() const
{
	return m_llPrePtr;
}

HINT64 HDLinkedListNode::self() const
{
	return m_llSelfPtr;
}

HUINT32 HDLinkedListNode::size()
{
	HUINT32 s = sizeof(HUINT8)
		+sizeof(m_ucFlag)
		+sizeof(m_ullTimestamp)
		+sizeof(m_lExpireMinutes)
		+sizeof(m_llNextPtr)
		+sizeof(m_llPrePtr)
		+sizeof(m_llSelfPtr)
		+sizeof(HUINT32)
		+sizeof(HUINT32)
		+m_strKey.length()
		+m_strValue.length();
	return s;
}

HUINT32 HDLinkedListNode::max_size()
{
	HUINT32 s = sizeof(HUINT8)
		+sizeof(HUINT8)
		+sizeof(HUINT64)
		+sizeof(HINT32)
		+sizeof(HINT64)
		+sizeof(HINT64)
		+sizeof(HINT64)
		+sizeof(HUINT32)
		+sizeof(HUINT32)
		+Global::MAX_KEY_LENGTH
		+Global::MAX_VALUE_LENGTH;
	return s;
}

HUINT32 HDLinkedListNode::min_size()
{
	HUINT32 s = sizeof(HUINT8)
		+sizeof(HUINT8)
		+sizeof(HUINT64)
		+sizeof(HINT32)
		+sizeof(HINT64)
		+sizeof(HINT64)
		+sizeof(HINT64)
		+sizeof(HUINT32)
		+sizeof(HUINT32);

	return s;
}

void HDLinkedListNode::set_self(HINT64 s)
{
	m_llSelfPtr = s;
}

void HDLinkedListNode::set_next(HINT64 n)
{
	m_llNextPtr = n;
}

void HDLinkedListNode::set_pre(HINT64 p)
{
	m_llPrePtr = p;
}

void HDLinkedListNode::set_value(const string &v)
{
	m_strValue = v;
}

HINT32 HDLinkedListNode::get_expire_minutes() const
{
	return m_lExpireMinutes;
}