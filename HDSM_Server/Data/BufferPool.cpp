#include "BufferPool.h"
#include "Utils.h"
#include <string.h>

BufferPool::BufferPool(HUINT32 nCount, HUINT32 nSize)
{
	if (nCount > 0 && nSize > 0)
	{
		m_pBuffer = new HCHAR[nCount*nSize];
		memset(m_pBuffer, 0, nCount*nSize);
		m_nSize = nSize;
		m_nCount = nCount;
		for (HUINT32 i=0; i<m_nCount; i++)
			m_unused.push(i);
	}
}

BufferPool::~BufferPool(void)
{
	if (m_pBuffer != NULL)
		delete m_pBuffer;
}

HINT32 BufferPool::lease()
{
	HINT32 index = -1;
	m_lock.lock();
	if (m_unused.size() > 0)
	{
		index = m_unused.front();
		m_unused.pop();
	}
	m_lock.unlock();
	return index;
}

void BufferPool::ret(HUINT32 iIndex)
{
	m_lock.lock();
	if (iIndex >= 0 && iIndex <m_nCount)
		m_unused.push(iIndex);
	m_lock.unlock();
	return;
}

void *BufferPool::get(HUINT32 iIndex)
{
	return (void *)(m_pBuffer+iIndex*m_nSize);
}

HUINT32 BufferPool::size()
{
	return m_unused.size();
}