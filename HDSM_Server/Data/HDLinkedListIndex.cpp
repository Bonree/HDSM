#include "HDLinkedListIndex.h"
#include <stdio.h>
#include <string.h>

HDLinkedListIndex::HDLinkedListIndex(IHDLinkedList *pList, HBOOL bEnableHighLevelIndex)
{
	m_pList = pList;
	m_bEnableHighLevelIndex = bEnableHighLevelIndex;
}

HDLinkedListIndex::~HDLinkedListIndex(void)
{
	clear();
}

HDLIST_INDEX_LEVEL HDLinkedListIndex::get_top_level()
{
	return m_bEnableHighLevelIndex ? Level_5 : Level_4;
}

void HDLinkedListIndex::update(map<string, interval> &m, HUINT32 level, const string &k, HINT64 offset, HBOOL init)
{
	if (m_pList == NULL)
		return;

	string key = k.substr(0, level);
	if (m.count(key) == 0)
	{
		interval val(offset, offset);
		m[key] = val;
	}
	else
	{
		map<string, interval>::iterator it = m.find(key);
		if (!init)
		{
			HDLinkedListNode node1 = m_pList->get_node((*it).second.llStartOffset);
			if (node1.is_valid() && node1.key().compare(k) > 0)
				(*it).second.llStartOffset = offset;

			HDLinkedListNode node2 = m_pList->get_node((*it).second.llEndOffset);
			if (node2.is_valid() && node2.key().compare(k) < 0)
				(*it).second.llEndOffset = offset;
		}
		else
		{
			(*it).second.llEndOffset = offset;
		}
	}

	return;
}

void HDLinkedListIndex::insert(const string &k, HINT64 offset, HBOOL init/* = false*/)
{
	update(m_Intervals1, Level_1, k, offset, init);
	update(m_Intervals2, Level_2, k, offset, init);
	update(m_Intervals3, Level_3, k, offset, init);
	update(m_Intervals4, Level_4, k, offset, init);
	if (m_bEnableHighLevelIndex)
		update(m_Intervals5, Level_5, k, offset, init);
	return;
}

interval HDLinkedListIndex::get(const string &k, HINT64 offset, HUINT32 &level)
{
	level = Level_None;
	interval val;
	string key5 = k.substr(0, Level_5);
	if (m_bEnableHighLevelIndex && m_Intervals5.count(key5) > 0)
	{
		val = (*(m_Intervals5.find(key5))).second;
		level = Level_5;
	}
	else
	{
		string key4 = k.substr(0, Level_4);
		if (m_Intervals4.count(key4) > 0)
		{
			val = (*(m_Intervals4.find(key4))).second;
			level = Level_4;
		}
		else
		{
			string key3 = k.substr(0, Level_3);
			if (m_Intervals3.count(key3) > 0)
			{
				val = (*(m_Intervals3.find(key3))).second;
				level = Level_3;
			}
			else
			{
				string key2 = k.substr(0, Level_2);
				if (m_Intervals2.count(key2) > 0)
				{
					val = (*(m_Intervals2.find(key2))).second;
					level = Level_2;
				}
				else
				{
					string key1 = k.substr(0, Level_1);
					if (m_Intervals1.count(key1) > 0)
					{
						val = (*(m_Intervals1.find(key1))).second;
						level = Level_1;
					}
				}
			}
		}
	}

	return val;
}

interval HDLinkedListIndex::query(map<string, interval> &m, HUINT32 level, const string &k)
{
	interval val;
	string key = k.substr(0, level);
	if (m.count(key) > 0)
		val = (*(m.find(key))).second;
	return val;
}

interval HDLinkedListIndex::query(const string &k, HINT32 level)
{
	interval val;
	if (level == Level_1)
		val = query(m_Intervals1, Level_1, k);
	else if (level == Level_2)
		val = query(m_Intervals2, Level_2, k);
	else if (level == Level_3)
		val = query(m_Intervals3, Level_3, k);
	else if (level == Level_4)
		val = query(m_Intervals4, Level_4, k);
	else if (m_bEnableHighLevelIndex && level == Level_5)
		val = query(m_Intervals5, Level_5, k);
	
	return val;
}

void HDLinkedListIndex::erase(map<string, interval> &m, HUINT32 level, HDLinkedListNode *pNode)
{
	string key = pNode->key().substr(0, level);
	if (m.count(key) > 0)
	{
		if (m[key].llStartOffset == m[key].llEndOffset)
		{
			m.erase(m.find(key));
		}
		else
		{
			if (pNode->self() == m[key].llStartOffset)
				m[key].llStartOffset = pNode->next();

			if (pNode->self() == m[key].llEndOffset)
				m[key].llEndOffset = pNode->pre();
		}
	}

	return;
}

void HDLinkedListIndex::erase(HDLinkedListNode *pNode)
{
	erase(m_Intervals1, Level_1, pNode);
	erase(m_Intervals2, Level_2, pNode);
	erase(m_Intervals3, Level_3, pNode);
	erase(m_Intervals4, Level_4, pNode);
	erase(m_Intervals5, Level_5, pNode);
	return;
}

void HDLinkedListIndex::clear()
{
	m_Intervals1.clear();
	m_Intervals2.clear();
	m_Intervals3.clear();
	m_Intervals4.clear();
	m_Intervals5.clear();
	return;
}

vector<HUINT32> HDLinkedListIndex::size()
{
	vector<HUINT32> vec;
	vec.push_back(m_Intervals1.size());
	vec.push_back(m_Intervals2.size());
	vec.push_back(m_Intervals3.size());
	vec.push_back(m_Intervals4.size());
	vec.push_back(m_Intervals5.size());
	return vec;
}