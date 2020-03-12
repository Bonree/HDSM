#pragma once
#include "Connection.h"
#include "SimpleLock.h"
#include "IBasePool.h"
#include <queue>
#include <vector>
#include <map>
#include "TypeDefine.h"

using namespace std;

class ConnectionPool 
	: public IBasePool
{
public:
	ConnectionPool(void);
	~ConnectionPool(void);
public:
	virtual HUINT32 size();
	virtual HINT32 lease();
	virtual void *get(HUINT32 index);
	virtual void ret(HUINT32 index);
public:
	vector<string> get_client_ips();
	HUINT32 get_used_size(); 
private:
	Connection m_Conns[FD_SETSIZE];
	SimpleLock m_lock;
	queue<HINT32> m_unusedConns;
	map<HINT32, HINT32> m_usedConns;
};

