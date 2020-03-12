#ifdef WIN32
#include <WinSock2.h>  
#endif
#include "HDSimpleMap.h"
#include "Utils.h"
#include "../ConfigureMgr.h"
#include "../Logger.h"

HUINT32 INIT_THREAD_COUNT = ConfigureMgr::get_init_threads_count();
HUINT32 KEY_PREFIX_LEN = 8;

HDSimpleMap::HDSimpleMap(const string &strDataPath)
{
	Utils::create_dirs(strDataPath);
	INIT_THREAD_COUNT = INIT_THREAD_COUNT/2 * 2;
	Logger::log_i("Data Shards: %d, Init Threads: %d", Global::MAX_SHARD_COUNT, INIT_THREAD_COUNT);
	m_ppList = new HDLinkedList *[Global::MAX_SHARD_COUNT];
	for (HUINT64 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		m_ppList[i] = new HDLinkedList(strDataPath+"/shard"+std::to_string(i+1)+".hdl");
	}

	m_nSuccessedInitThreadCount = 0;
	HUINT32 interval = 0;
	if (Global::MAX_SHARD_COUNT%INIT_THREAD_COUNT == 0)
		interval = (Global::MAX_SHARD_COUNT/INIT_THREAD_COUNT);
	else
		interval = (Global::MAX_SHARD_COUNT/INIT_THREAD_COUNT)+1;
	for (HUINT32 i=0; i<INIT_THREAD_COUNT; i++)
	{
		HUINT32 nStartIndex = interval*i;
		HUINT32 nEndIndex = interval*(i+1);
		HDSMInitThread *pThd = new HDSMInitThread(this, nStartIndex, nEndIndex);
		if (pThd != NULL)
		{
			m_vecThreads.push_back(pThd);
			pThd->start();
		}	
	}

	m_Notifier.wait();//等待所有初始化线程完成

	//启动清理过期key线程
	m_pTrimExpireThread = new TrimExpireThread(this);
	if (m_pTrimExpireThread != NULL)
		m_pTrimExpireThread->start();
	
}

HDSimpleMap::~HDSimpleMap(void)
{
	if (m_pTrimExpireThread != NULL)
	{
		m_pTrimExpireThread->stop();
		delete m_pTrimExpireThread;
		m_pTrimExpireThread = NULL;
	}

	for (HUINT32 i=0; i<m_vecThreads.size(); i++)
	{
		HDSMInitThread *pThd = m_vecThreads[i];
		if (pThd != NULL)
		{
			pThd->stop();
			delete pThd;
		}
	}
	m_vecThreads.clear();

	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
			delete m_ppList[i];
	}

	if (m_ppList != NULL)
		delete [] m_ppList;

}

HBOOL HDSimpleMap::get(const string &k, string &v, HINT32 &lExpireMinutes)
{
	string fk = get_format_key(k);
	return m_ppList[Utils::hash(fk, KEY_PREFIX_LEN)%Global::MAX_SHARD_COUNT]->get(fk, v, lExpireMinutes);
}

HBOOL HDSimpleMap::put(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	string fk = get_format_key(k);
	return m_ppList[Utils::hash(fk, KEY_PREFIX_LEN)%Global::MAX_SHARD_COUNT]->put(fk, v, ts, lExpireMinutes);
}

HBOOL HDSimpleMap::update(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	string fk = get_format_key(k);
	return m_ppList[Utils::hash(fk, KEY_PREFIX_LEN)%Global::MAX_SHARD_COUNT]->update(fk, v, ts, lExpireMinutes);
}

HBOOL HDSimpleMap::erase(const string &k)
{
	string fk = get_format_key(k);
	return m_ppList[Utils::hash(fk, KEY_PREFIX_LEN)%Global::MAX_SHARD_COUNT]->erase(fk);
}

HBOOL HDSimpleMap::exists(const string &k)
{
	string fk = get_format_key(k);
	return m_ppList[Utils::hash(fk, KEY_PREFIX_LEN)%Global::MAX_SHARD_COUNT]->exists(fk);
}

HUINT32 HDSimpleMap::length()
{
	HUINT32 len = 0;
	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
			len += m_ppList[i]->length();
	}
	return len;
}

string HDSimpleMap::get_format_key(const string &k)
{
	static const HCHAR *TABLE = "0123456789abcdef";
	HUINT32 hash = Utils::hash(k, k.size());
	string str;
	str += TABLE[hash & 0x0000000F];
	str += TABLE[(hash>>4) & 0x0000000F];
	str += TABLE[(hash>>8) & 0x0000000F];
	str += TABLE[(hash>>12) & 0x0000000F];
	str += TABLE[(hash>>16) & 0x0000000F];
	str += TABLE[(hash>>20) & 0x0000000F];
	str += TABLE[(hash>>24) & 0x0000000F];
	str += TABLE[(hash>>28) & 0x0000000F];
	str += "@";
	str += k;
	return str;
}

string HDSimpleMap::get_raw_key(const string &k)
{
	string str;
	HINT32 p = k.find("@");
	if (p != -1)
		str = k.substr(p+1, k.size()-p-1);
	return str;
}

HUINT32 HDSimpleMap::expire()
{
	HUINT32 ulCount = 0;
	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
			ulCount += m_ppList[i]->trim_for_expire();
	}
	return ulCount;
}

vector<HUINT32> HDSimpleMap::get_indexs_size()
{
	vector<HUINT32> vecSizeInfo;
	HUINT32 len[5] = {0};
	
	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
		{
			vector<HUINT32> vec = m_ppList[i]->get_indexs_size();
			for (HUINT32 j=0; j<vec.size(); j++)
				len[j] += vec[j];
		}
	}

	for (HINT32 k=0; k<5; k++)
		vecSizeInfo.push_back(len[k]);

	return vecSizeInfo;
}

HBOOL HDSimpleMap::init_some_hdlists(HUINT32 nStartIndex, HUINT32 nEndIndex)
{
	if (m_ppList != NULL)
	{
		for (HUINT32 i=nStartIndex; i<nEndIndex; i++)
		{
			if (i < Global::MAX_SHARD_COUNT && m_ppList[i] != NULL)
				m_ppList[i]->init();
		}
	}

	m_lock.lock();
	m_nSuccessedInitThreadCount++;
	if (m_nSuccessedInitThreadCount >= INIT_THREAD_COUNT)
		m_Notifier.awake();//全部初始化线程完成，发通知
	m_lock.unlock();

	return true;
}

HBOOL HDSimpleMap::load_expire_cache()
{
	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
			m_ppList[i]->load_expire_cache();
	}
	return true;
}

HUINT32 HDSimpleMap::get_expire_cache_size()
{
	HUINT32 size = 0;
	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
		{
			size += m_ppList[i]->get_expire_cache_size();
		}
	}
	return size;
}

map<string, KeyValueInfo> HDSimpleMap::keys(HUINT32 limit)
{
	map<string, KeyValueInfo> kvs;
	for (HUINT32 i=0; i<Global::MAX_SHARD_COUNT; i++)
	{
		if (m_ppList[i] != NULL)
		{
			map<string, KeyValueInfo> m;
			m_ppList[i]->output_to_map(m, limit/Global::MAX_SHARD_COUNT+16);
			map<string, KeyValueInfo>::iterator iter;
			for (iter=m.begin(); iter!=m.end(); iter++)
			{
				kvs.insert(std::make_pair(get_raw_key((*iter).first), (*iter).second));
				if (kvs.size() >= limit)
					return kvs;
			}
		}
	}
	return kvs;
}