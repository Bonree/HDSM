#pragma once
#include <vector>
#include "HDLinkedList.h"
#include "IHDSMEventHandler.h"
#include "../Global.h"
#include "SimpleNotifier.h"
#include "SimpleLock.h"
#include "TrimExpireThread.h"
#include "HDSMInitThread.h"
#include "HDSMOperateType.h"
#include "TypeDefine.h"
using namespace std;

class HDSimpleMap 
	: public IHDSMEventHandler
{
public:
	HDSimpleMap(const string &strDataPath);
	virtual ~HDSimpleMap(void);
public:
	HBOOL get(const string &k, string &v, HINT32 &lExpireMinutes);
	HBOOL put(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes = -1);
	HBOOL update(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes = 0);
	HBOOL erase(const string &k);
	HBOOL exists(const string &k);
	virtual HUINT32 length();
	map<string, KeyValueInfo> keys(HUINT32 limit);
public:
	virtual HUINT32 expire();
	virtual HBOOL init_some_hdlists(HUINT32 nStartIndex, HUINT32 nEndIndex);
	virtual HBOOL load_expire_cache();
public:
	HUINT32 get_max_key_length();
	HUINT32 get_max_value_length();
	vector<HUINT32> get_indexs_size();
	HUINT32 get_expire_cache_size();
private:
	string get_format_key(const string &k);
	string get_raw_key(const string &k);
private:
	HDLinkedList	**m_ppList;
	SimpleNotifier	m_Notifier;
	TrimExpireThread *m_pTrimExpireThread;
	SimpleLock	m_lock;
	HUINT32		m_nSuccessedInitThreadCount;
	vector<HDSMInitThread *> m_vecThreads;
};

