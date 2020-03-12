#include "Result.h"
#include "Task.h"
#include "ResultImpl.h"
#include <string.h>
#include "Utils.h"
#include "TypeDefine.h"

Result::Result(const string &k, const string &v)
{
	Impl = (void *)new ResultImpl;
	((ResultImpl *)Impl)->m_bValid = false;
	((ResultImpl *)Impl)->m_strKey = k;
	((ResultImpl *)Impl)->m_strValue = v;
}

Result::Result(const string &k)
{
	Impl = (void *)new ResultImpl;
	((ResultImpl *)Impl)->m_bValid = false;
	((ResultImpl *)Impl)->m_strKey = k;
}

Result::Result()
{
	Impl = (void *)new ResultImpl;
	((ResultImpl *)Impl)->m_bValid = false;
}

Result::~Result(void)
{
}

HBOOL Result::load(const HCHAR *pBuf, HUINT32 nBufLen)
{
	//checksum(1)
	//bRet(4)
	//sn(4)
	//op(4)
	//totaltime(4)
	//processtime(4)
	//errinfolen(4)
	//errinfo(errinfolen)
	//+[length(4)]+[valuelen(4)+value]+expireMinutes(4)
	if (pBuf != NULL && nBufLen > 0)
	{
		//HUINT32 ulValueLen = 0;
		HUINT32 offset = sizeof(HUINT8);
		HUINT8 checksum = pBuf[0];
		if (checksum == Utils::get_check_sum(pBuf+1, nBufLen-1))
		{
			memcpy(&((ResultImpl *)Impl)->m_bRet, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			memcpy(&((ResultImpl *)Impl)->m_ulSN, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			memcpy(&((ResultImpl *)Impl)->m_ulOperate, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			memcpy(&((ResultImpl *)Impl)->m_ulTotalTime, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			memcpy(&((ResultImpl *)Impl)->m_ulProcessTime, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			HUINT32 ulErrInfoLen = 0;
			memcpy(&ulErrInfoLen, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			if (ulErrInfoLen > 0)
			{
				((ResultImpl *)Impl)->m_strErrInfo.append(&pBuf[offset], ulErrInfoLen);
				offset += ulErrInfoLen;
			}

			((ResultImpl *)Impl)->m_bValid = true;

			if (((ResultImpl *)Impl)->m_ulOperate > OP_BEGIN && ((ResultImpl *)Impl)->m_ulOperate < OP_END)
			{	
				//HUINT32 ulKeyLen = 0;
				HUINT32 ulValueLen = 0;
				switch (((ResultImpl *)Impl)->m_ulOperate)
				{
				case OP_PUT:
				case OP_ERASE:
				case OP_EXISTS:
				case OP_SHUTDOWN:
				case OP_UPDATE:
					break;
				case OP_GET:	
					if (((ResultImpl *)Impl)->m_bRet)
					{
						memcpy(&ulValueLen, pBuf+offset, sizeof(HUINT32));
						offset += sizeof(HUINT32);

						((ResultImpl *)Impl)->m_strValue.append(&pBuf[offset], ulValueLen);
						offset += ulValueLen;

						memcpy(&((ResultImpl *)Impl)->m_lExpireMinutes, pBuf+offset, sizeof(HINT32));
						offset += sizeof(HINT32);
					}
					break;
				case OP_ECHO:
				case OP_QUIT:
					memcpy(&((ResultImpl *)Impl)->m_ulMaxKeyLength, pBuf+offset, sizeof(HUINT32));
					offset += sizeof(HUINT32);

					memcpy(&((ResultImpl *)Impl)->m_ulMaxValueLength, pBuf+offset, sizeof(HUINT32));
					offset += sizeof(HUINT32);
					break;
				case OP_LENGTH:
					if (((ResultImpl *)Impl)->m_bRet)
					{
						memcpy(&((ResultImpl *)Impl)->m_ulLength, pBuf+offset, sizeof(HUINT32));
						offset += sizeof(HUINT32);
					}
					break;
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
				case OP_KEYS:
					if (((ResultImpl *)Impl)->m_bRet)
					{
						HUINT32 ulKeyCount = 0;
						memcpy(&ulKeyCount, pBuf+offset, sizeof(HUINT32));
						offset += sizeof(HUINT32);

						for (HUINT32 i=0; i<ulKeyCount; i++)
						{
							KeyValueInfo kv;

							HUINT32 ulKeyLen = 0;
							memcpy(&ulKeyLen, pBuf+offset, sizeof(HUINT32));
							offset += sizeof(HUINT32);

							kv.key.append(&pBuf[offset], ulKeyLen);
							offset += ulKeyLen;

							HUINT32 ulValueLen = 0;
							memcpy(&ulValueLen, pBuf+offset, sizeof(HUINT32));
							offset += sizeof(HUINT32);

							kv.value.append(&pBuf[offset], ulValueLen);
							offset += ulValueLen;

							memcpy(&kv.expire_minutes, pBuf+offset, sizeof(HINT32));
							offset += sizeof(HINT32);

							memcpy(&kv.created_time, pBuf+offset, sizeof(HUINT64));
							offset += sizeof(HUINT64);

							((ResultImpl *)Impl)->m_vecKeysInfo.push_back(kv);
						}
					}
					break;
				default:
					break;
				}
			}
		}
	}

	return ((ResultImpl *)Impl)->m_bValid;
}

const string &Result::get_value()
{
	return ((ResultImpl *)Impl)->m_strValue;
}

HUINT32 Result::get_length()
{
	return ((ResultImpl *)Impl)->m_ulLength;
}

OPRERATE_TYPE Result::get_operate()
{
	return (OPRERATE_TYPE)((ResultImpl *)Impl)->m_ulOperate;
}

HUINT32 Result::get_total_time()
{
	return ((ResultImpl *)Impl)->m_ulTotalTime;
}

HUINT32 Result::get_process_time()
{
	return ((ResultImpl *)Impl)->m_ulProcessTime;
}

HBOOL Result::is_valid()
{
	return ((ResultImpl *)Impl)->m_bValid;
}

HUINT32 Result::get_max_key_length()
{
	return ((ResultImpl *)Impl)->m_ulMaxKeyLength;
}

HUINT32 Result::get_max_value_length()
{
	return ((ResultImpl *)Impl)->m_ulMaxValueLength;
}

void Result::reset()
{
	((ResultImpl *)Impl)->m_bValid = false;
	((ResultImpl *)Impl)->m_strKey.empty();
	((ResultImpl *)Impl)->m_strValue.empty();
	((ResultImpl *)Impl)->m_strErrInfo.empty();
	((ResultImpl *)Impl)->m_vecKeysInfo.empty();
	((ResultImpl *)Impl)->m_ulSN = 0;
}

HBOOL Result::get_ret()
{
	return ((ResultImpl *)Impl)->m_bRet == 0 ? false : true;
}

HUINT32 Result::get_SN()
{
	return ((ResultImpl *)Impl)->m_ulSN;
}

HINT32 Result::get_expire_minutes()
{
	return ((ResultImpl *)Impl)->m_lExpireMinutes;
}

const string &Result::get_err_info()
{
	return ((ResultImpl *)Impl)->m_strErrInfo;
}

vector<KeyValueInfo> Result::get_keys_info()
{
	return ((ResultImpl *)Impl)->m_vecKeysInfo;
}