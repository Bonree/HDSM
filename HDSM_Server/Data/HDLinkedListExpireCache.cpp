#include "HDLinkedListExpireCache.h"
#include "Utils.h"
#include "../Global.h"

HDLinkedListExpireCache::HDLinkedListExpireCache(void)
{
}


HDLinkedListExpireCache::~HDLinkedListExpireCache(void)
{
	m_NearExpiredKeys.clear();
}


HBOOL HDLinkedListExpireCache::post(const HDLinkedListNode *pNode)
{
	if (pNode == NULL)
		return false;

	if (pNode->get_expire_minutes() < 0)
		return true;

	HUINT64 ullMaxTimestamp = Utils::get_current_time_stamp() + Global::SECONDS_IN_ONE_DAY;
	HUINT64 ullExpiredTimestamp = pNode->get_time_stamp() + pNode->get_expire_minutes()*Global::SECONDS_IN_ONE_MINUTE;

	//key精确到下一分钟
	if (ullExpiredTimestamp < ullMaxTimestamp)
	{
		HUINT64 ulExpiredTS = Utils::get_time_stamp_align_up_minute(ullExpiredTimestamp);

		if (m_NearExpiredKeys.count(ulExpiredTS) > 0)
		{
			m_NearExpiredKeys[ulExpiredTS].push_back(pNode->self());
		}
		else
		{
			vector<HUINT32> vec;
			vec.push_back(pNode->self());
			m_NearExpiredKeys[ulExpiredTS] = vec;
		}
	}

	return true;
}

void HDLinkedListExpireCache::clear()
{
	m_NearExpiredKeys.clear();
	return;
}

vector<vector<HUINT32>> HDLinkedListExpireCache::peek()
{
	vector<vector<HUINT32>> vec;
	HUINT64 ullCurrentTS = Utils::get_current_time_stamp_align_minute();

	map<HUINT64, vector<HUINT32>>::iterator it;
	for (it = m_NearExpiredKeys.begin(); it != m_NearExpiredKeys.end(); )
	{
		if ((*it).first <= ullCurrentTS)
		{
			vec.push_back((*it).second);
			it = m_NearExpiredKeys.erase(it);
		}
		else
			break;
	}

	return vec;
}

HUINT32 HDLinkedListExpireCache::size()
{
	HUINT32 size = 0;
	map<HUINT64, vector<HUINT32>>::iterator it;
	for (it = m_NearExpiredKeys.begin(); it != m_NearExpiredKeys.end(); it++)
	{
		size += (*it).second.size();
	}
	return size;
}