#ifdef WIN32
#include <WinSock2.h>  
#endif
#include "Task.h"
#include "Utils.h"
#include "../Global.h"
#include <string.h>

Task::Task()
{
	m_bValid = false;
}

Task::Task(const Task &t)
{
	m_bValid = t.m_bValid;
	m_bRet = t.m_bRet;
	m_strKey = t.m_strKey;
	m_strValue = t.m_strValue;
	m_ulLength = t.m_ulLength;
	m_ulOperate = t.m_ulOperate;
	m_ulSN = t.m_ulSN;
	m_lExpireMinutes = t.m_lExpireMinutes;
	m_strPassword = t.m_strPassword;
	m_pConn = t.m_pConn;
	m_ulRecvMoment = t.m_ulRecvMoment;
	m_ulStartMoment = t.m_ulStartMoment;
	m_ulFinishMoment = t.m_ulFinishMoment;
	m_ullTimeStamp = t.m_ullTimeStamp;
	m_ullAckIndex = t.m_ullAckIndex;
	m_strSyncData = t.m_strSyncData;
	m_strErrInfo = t.m_strErrInfo;
	m_KeysInfo = t.m_KeysInfo;
	m_ulShowKeysLimit = t.m_ulShowKeysLimit;
}

Task::Task(Connection *pConn, const HCHAR *pszBuf, HUINT32 iBufLen)
{
	m_bValid = false;
	//checksum(1)+sn(4)+op(4)+[keylen(4)+key(keylen)]+[valuelen(4)+value(valuelen)]+expireminutes(4)
	//checksum(1)+sn(4)+op(4)+ackindex(8)
	//checksum(1)+sn(4)+op(4)+passwordlen(4)+password(passwordlen)
	//checksum(1)+sn(4)+op(4)+limit(4)
	if (pszBuf != NULL && iBufLen > 0)
	{
		HUINT32 offset = sizeof(HUINT8);
		HUINT8 checksum = pszBuf[0];
		if (checksum == Utils::get_check_sum(pszBuf+sizeof(HUINT8), iBufLen-sizeof(HUINT8)))
		{
			m_ullTimeStamp = Utils::get_current_time_stamp();

			memcpy(&m_ulSN, pszBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			memcpy(&m_ulOperate, pszBuf+offset, sizeof(HUINT32));
			if (m_ulOperate > OP_BEGIN && m_ulOperate < OP_END)
			{
				offset += sizeof(HUINT32);
				HUINT32 ulKeyLen = 0;
				HUINT32 ulValueLen = 0;
				HUINT32 ulPwdLen = 0;
				switch (m_ulOperate)
				{
				case OP_ECHO:
				case OP_LENGTH:
				case OP_SYNC:
				case OP_QUIT:
					m_ulRecvMoment = Utils::get_tick_count();
					m_pConn = pConn;
					m_bValid = true;
					break;
				case OP_ACK:
					memcpy(&m_ullAckIndex, pszBuf+offset, sizeof(HUINT64));
					offset += sizeof(HUINT64);
					m_ulRecvMoment = Utils::get_tick_count();
					m_pConn = pConn;
					m_bValid = true;
					break;
				case OP_GET:	
				case OP_ERASE:
				case OP_EXISTS:
					memcpy(&ulKeyLen, pszBuf+offset, sizeof(HUINT32));
					if (ulKeyLen > 0 && ulKeyLen < Global::MAX_KEY_LENGTH)
					{
						offset += sizeof(HUINT32);
						m_strKey.append(&pszBuf[offset], ulKeyLen);
						m_ulRecvMoment = Utils::get_tick_count();
						m_pConn = pConn;
						m_bValid = true;
					}
					break;
				case OP_SHUTDOWN:
					memcpy(&ulPwdLen, pszBuf+offset, sizeof(HUINT32));
					if (ulPwdLen > 0)
					{
						offset += sizeof(HUINT32);
						m_strPassword.append(&pszBuf[offset], ulPwdLen);
						m_ulRecvMoment = Utils::get_tick_count();
						m_pConn = pConn;
						m_bValid = true;
					}
					break;
				case OP_PUT:
				case OP_UPDATE:
					memcpy(&ulKeyLen, pszBuf+offset, sizeof(HUINT32));
					if (ulKeyLen > 0 && ulKeyLen < Global::MAX_KEY_LENGTH)
					{
						offset += sizeof(HUINT32);

						m_strKey.append(&pszBuf[offset], ulKeyLen);
						offset += ulKeyLen;
						
						memcpy(&ulValueLen, &pszBuf[offset], sizeof(HUINT32));
						if (ulValueLen > 0 && ulValueLen < Global::MAX_VALUE_LENGTH)
						{
							offset += sizeof(HUINT32);

							m_strValue.append(&pszBuf[offset], ulValueLen);
							offset += ulValueLen;

							memcpy(&m_lExpireMinutes, &pszBuf[offset], sizeof(HINT32));
							offset += sizeof(HINT32);

							m_ulRecvMoment = Utils::get_tick_count();
							m_pConn = pConn;
							m_bValid = true;

							//在从oplog读到数据中会多一个ts字段
							if (offset < iBufLen)
							{
								memcpy(&m_ullTimeStamp, &pszBuf[offset], sizeof(HUINT64));
								offset += sizeof(HUINT64);
							}
						}
					}
					break;
				case OP_KEYS:
					memcpy(&m_ulShowKeysLimit, pszBuf+offset, sizeof(HUINT32));
					offset += sizeof(HUINT32);
					m_ulRecvMoment = Utils::get_tick_count();
					m_pConn = pConn;
					m_bValid = true;
					break;
				default:
					break;
				}
			}
		}
	}
}

Task::~Task(void)
{
	m_strKey.clear();
	m_strValue.clear();
	m_strSyncData.clear();
	m_KeysInfo.clear();
}

HBOOL Task::is_valid() const
{
	return m_bValid;
}

HINT32 Task::get_expire_minutes()
{
	return m_lExpireMinutes;
}

const string &Task::get_key()
{
	return m_strKey;
}

HUINT32 Task::get_show_keys_limit()
{
	return m_ulShowKeysLimit;
}

void Task::set_expire_minutes(HINT32 lExpireMinutes)
{
	m_lExpireMinutes = lExpireMinutes;
}

const string &Task::get_value()
{
	return m_strValue;
}

OPRERATE_TYPE Task::get_operate()
{
	return (OPRERATE_TYPE)m_ulOperate;
}

string Task::get_operate_str() const
{
	string s = "UNKNOWN";
	switch (m_ulOperate)
	{
	case OP_ECHO:
		s = "ECHO";
		break;
	case OP_QUIT:
		s = "QUIT";
		break;
	case OP_GET:
		s = "GET";
		break;
	case OP_PUT:
		s = "PUT";
		break;
	case  OP_UPDATE:
		s = "UPDATE";
		break;
	case OP_ERASE:
		s = "ERASE";
		break;
	case OP_EXISTS:
		s = "EXISTS";
		break;
	case OP_LENGTH:
		s = "LENGTH";
		break;
	case OP_ACK:
		s = "ACK";
		break;
	case OP_SYNC:
		s = "SYNC";
		break;
	case OP_SHUTDOWN:
		s = "SHUTDOWN";
		break;
	case OP_KEYS:
		s = "KEYS";
		break;
	default:
		s = "UNKNOWN";
		break;
	}
	return s;
}

HUINT32 Task::get_length()
{
	return m_ulLength;
}

void Task::set_ret(HBOOL r)
{
	m_bRet = r?1:0;
	m_ulFinishMoment = Utils::get_tick_count();
}

void Task::set_value(const string &v)
{
	m_strValue = v;
}

void Task::set_length(HUINT32 len)
{
	m_ulLength = len;
}

Connection *Task::get_conn()
{
	return m_pConn;
}

HUINT32 Task::get_total_time()
{
	return m_ulFinishMoment-m_ulRecvMoment;
}

HUINT32 Task::get_process_time()
{
	return m_ulFinishMoment-m_ulStartMoment;
}

HUINT32 Task::get_sn()
{
	return m_ulSN;
}

void Task::record_start_moment()
{
	m_ulStartMoment = Utils::get_tick_count();
}

string Task::buffer()
{
	HUINT32 bl = 0;
	string str;
	HUINT32 ulTotalTime = get_total_time();
	HUINT32 ulProcessTime = get_process_time();
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	if (m_ulOperate == OP_PUT || m_ulOperate == OP_UPDATE || m_ulOperate == OP_ERASE || m_ulOperate == OP_EXISTS || m_ulOperate == OP_ACK || m_ulOperate == OP_SHUTDOWN)
	{
		bl = sizeof(HUINT8) 
			+ sizeof(HUINT32) 
			+ sizeof(HUINT32) 
			+ sizeof(HUINT32) 
			+ sizeof(HUINT32) 
			+ sizeof(HUINT32)
			+ sizeof(HUINT32)
			+ m_strErrInfo.length();
		
		str.append((const HCHAR *)&bl, sizeof(HUINT32));

		string _str;
		_str.append((const HCHAR *)&m_bRet, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulTotalTime, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulProcessTime, sizeof(HUINT32));

		HUINT32 ulErrInfoLen = m_strErrInfo.length();
		_str.append((const HCHAR *)&ulErrInfoLen, sizeof(HUINT32));
		_str += m_strErrInfo;
	
		HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
		str += checksum;

		str += _str;
	}
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	//ackindex(8)
	//[datalen(4)]
	//[data(datalen)]
	else if (m_ulOperate == OP_SYNC)
	{
		bl = sizeof(HUINT8) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			m_strErrInfo.length() +
			sizeof(HUINT64);

		if (m_bRet && m_strSyncData.size() > 0)
			bl += (sizeof(HUINT32) + m_strSyncData.size());

		str.append((const HCHAR *)&bl, sizeof(HUINT32));

		string _str;
		_str.append((const HCHAR *)&m_bRet, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulTotalTime, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulProcessTime, sizeof(HUINT32));
		
		HUINT32 ulErrInfoLen = m_strErrInfo.length();
		_str.append((const HCHAR *)&ulErrInfoLen, sizeof(HUINT32));
		_str += m_strErrInfo;

		_str.append((const HCHAR *)&m_ullAckIndex, sizeof(HUINT64));

		if (m_bRet && m_strSyncData.size() > 0)
		{
			HUINT32 ulDataLen = m_strSyncData.size();
			_str.append((const HCHAR *)&ulDataLen, sizeof(HUINT32));
			_str += m_strSyncData;
		}

		HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
		str += checksum;

		str += _str;
	}
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	//[valuelen(4)]
	//[value(valuelen)]
	//[expireminutes(4)]
	else if (m_ulOperate == OP_GET)
	{
		bl = sizeof(HUINT8) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			m_strErrInfo.length();

		if (m_bRet)
			bl += (sizeof(HUINT32) + m_strValue.size() + sizeof(HINT32));

		str.append((const HCHAR *)&bl, sizeof(HUINT32));

		string _str;
		_str.append((const HCHAR *)&m_bRet, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulTotalTime, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulProcessTime, sizeof(HUINT32));

		HUINT32 ulErrInfoLen = m_strErrInfo.length();
		_str.append((const HCHAR *)&ulErrInfoLen, sizeof(HUINT32));
		_str += m_strErrInfo;

		if (m_bRet)
		{
			HUINT32 ulValueLen = m_strValue.size();
			_str.append((const HCHAR *)&ulValueLen, sizeof(HUINT32));
			_str += m_strValue;

			_str.append((const HCHAR *)&m_lExpireMinutes, sizeof(HINT32));
		}

		HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
		str += checksum;

		str += _str;
	}
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	//[Length(4)]
	else if (m_ulOperate == OP_LENGTH)
	{
		bl = sizeof(HUINT8) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			m_strErrInfo.length();

		if (m_bRet)
			bl += sizeof(HUINT32);

		str.append((const HCHAR *)&bl, sizeof(HUINT32));

		string _str;
		_str.append((const HCHAR *)&m_bRet, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulTotalTime, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulProcessTime, sizeof(HUINT32));

		HUINT32 ulErrInfoLen = m_strErrInfo.length();
		_str.append((const HCHAR *)&ulErrInfoLen, sizeof(HUINT32));
		_str += m_strErrInfo;
		
		if (m_bRet)
			_str.append((const HCHAR *)&m_ulLength, sizeof(HUINT32));

		HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
		str += checksum;

		str += _str;
	}
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	//keycount(4)
	//key1Len(4)
	//key1(key1Len)
	//value1Len(4)
	//value1(value1Len)
	//key1_expire_minutes(4)
	//key1_created_time(8)
	//key2Len(4)
	//key2(key1Len)
	//value2Len(4)
	//value2(value1Len)
	//key2_expire_minutes(4)
	//key2_created_time(8)
	else if (m_ulOperate == OP_KEYS)
	{
		bl = sizeof(HUINT8) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			m_strErrInfo.length() +
			sizeof(HUINT32);

		map<string, KeyValueInfo>::iterator iter;
		for (iter=m_KeysInfo.begin(); iter!=m_KeysInfo.end(); iter++)
		{
			bl += sizeof(HUINT32);
			bl += (*iter).first.length();
			bl += sizeof(HUINT32);
			bl += (*iter).second.value.length();
			bl += sizeof(HINT32);
			bl += sizeof(HUINT64);
		}

		str.append((const HCHAR *)&bl, sizeof(HUINT32));

		string _str;
		_str.append((const HCHAR *)&m_bRet, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulTotalTime, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulProcessTime, sizeof(HUINT32));

		HUINT32 ulErrInfoLen = m_strErrInfo.length();
		_str.append((const HCHAR *)&ulErrInfoLen, sizeof(HUINT32));
		_str += m_strErrInfo;

		HUINT32 ulKeyCount = m_KeysInfo.size();
		_str.append((const HCHAR *)&ulKeyCount, sizeof(HUINT32));

		for (iter=m_KeysInfo.begin(); iter!=m_KeysInfo.end(); iter++)
		{
			HUINT32 ulKeyLen = (*iter).first.length();
			_str.append((const HCHAR *)&ulKeyLen, sizeof(HUINT32));
			_str += (*iter).first;

			HUINT32 ulValueLen = (*iter).second.value.length();
			_str.append((const HCHAR *)&ulValueLen, sizeof(HUINT32));
			_str += (*iter).second.value;

			HINT32 lExpireMinutes = (*iter).second.expire_minutes;
			_str.append((const HCHAR *)&lExpireMinutes, sizeof(HINT32));

			HUINT64 ts = (*iter).second.created_time;
			_str.append((const HCHAR *)&ts, sizeof(HUINT64));
		}

		HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
		str += checksum;

		str += _str;
	}
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	//max_key_lenght(4)
	//max_value_length(4)
	else if (m_ulOperate == OP_ECHO || m_ulOperate == OP_QUIT)
	{
		bl = sizeof(HUINT8) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) + 
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			m_strErrInfo.length() +
			sizeof(HUINT32) + 
			sizeof(HUINT32);

		str.append((const HCHAR *)&bl, sizeof(HUINT32));
		
		string _str;
		_str.append((const HCHAR *)&m_bRet, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulSN, sizeof(HUINT32));
		_str.append((const HCHAR *)&m_ulOperate, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulTotalTime, sizeof(HUINT32));
		_str.append((const HCHAR *)&ulProcessTime, sizeof(HUINT32));

		HUINT32 ulErrInfoLen = m_strErrInfo.length();
		_str.append((const HCHAR *)&ulErrInfoLen, sizeof(HUINT32));
		_str += m_strErrInfo;

		_str.append((const HCHAR *)&Global::MAX_KEY_LENGTH, sizeof(HUINT32));
		_str.append((const HCHAR *)&Global::MAX_VALUE_LENGTH, sizeof(HUINT32));

		HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
		str += checksum;

		str += _str;
	}

	return str;
}

HUINT64 Task::get_time_stamp()
{
	return m_ullTimeStamp;
}

HUINT64 Task::get_ack_index()
{
	return m_ullAckIndex;
}

const string &Task::get_password()
{
	return m_strPassword;
}

void Task::set_sync_data(const string &data)
{
	m_strSyncData = data;
	return;
}

void Task::set_ack_index(HUINT64 ullAckIndex)
{
	m_ullAckIndex = ullAckIndex;
	return;
}

void Task::set_err_info(const string &info)
{
	m_strErrInfo = info;
	return;
}

void Task::set_keys_info(map<string, KeyValueInfo> &keys)
{
	m_KeysInfo = keys;
	return;
}