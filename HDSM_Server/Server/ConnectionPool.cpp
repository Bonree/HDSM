#ifdef WIN32
#include <WinSock2.h>  
#endif
#include "ConnectionPool.h"

ConnectionPool::ConnectionPool(void)
{
	for (HINT32 i=0; i<FD_SETSIZE; i++)
	{
		m_Conns[i].set_index(i);
		m_unusedConns.push(i);
	}
}

ConnectionPool::~ConnectionPool(void)
{
	m_unusedConns.empty();
	m_usedConns.empty();
}

HUINT32 ConnectionPool::size()
{
	return m_unusedConns.size();
}

HINT32 ConnectionPool::lease()
{
	HINT32 index = -1;
	m_lock.lock();
	if (m_unusedConns.size() > 0)
	{
		index = m_unusedConns.front();
		m_unusedConns.pop();

		m_usedConns[index] = index;
	}
	m_lock.unlock();
	return index;
}

void ConnectionPool::ret(HUINT32 index)
{
	if (index>=0 && index<FD_SETSIZE)
	{
		m_lock.lock();
		m_Conns[index].disabled();
		m_unusedConns.push(index);
		m_usedConns.erase(index);
		m_lock.unlock();
	}
	return;
}

void *ConnectionPool::get(HUINT32 index)
{
	Connection *pConn = NULL;
	if (index>=0 && index<FD_SETSIZE)
		pConn =  &m_Conns[index];
	return pConn;
}

vector<string> ConnectionPool::get_client_ips()
{
	vector<string> ips;
	map<HINT32, HINT32>::iterator iter;
	for (iter = m_usedConns.begin(); iter != m_usedConns.end(); iter++)
	{
		ips.push_back(((Connection *)get((*iter).first))->get_peer_ip());
	}
	return ips;
}

HUINT32 ConnectionPool::get_used_size()
{
	return m_usedConns.size();
}