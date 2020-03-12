#include "HDLinkedList.h"
#include "Utils.h"
#include "../Global.h"
#include "../Logger.h"
#include <string.h>

#define GET_NODE(_pNode_, _offset_)					\
	HDLinkedListNode _node_ = get_node(_offset_);	\
	if (_node_.is_valid())							\
		_pNode_ = &_node_;							\
	else											\
		_pNode_ = NULL;

HDLinkedList::HDLinkedList(const string &strDataFile) 
	:	FILE_TAG("HDLINKEDLIST"),
		OFFSET_LENGTH_FIELD(strlen(FILE_TAG)+sizeof(Global::VERSION_CODE)),
		OFFSET_STARTOFFSET_FIELD(strlen(FILE_TAG)+sizeof(Global::VERSION_CODE)+sizeof(m_ulLength)+sizeof(m_ulInvalidLength)),
		OFFSET_INVALID_LENGTH_FIELD(strlen(FILE_TAG)+sizeof(Global::VERSION_CODE)+sizeof(m_ulLength))
{
	m_bInitialized = false;
	m_llFileEndPos = 0;
	m_ulLength = 0;
	m_ulInvalidLength = 0;
	m_llStartOffset = strlen(FILE_TAG)+sizeof(Global::VERSION_CODE)+sizeof(m_ulLength)+sizeof(m_ulInvalidLength)+sizeof(m_llStartOffset);
	m_strDataFile = strDataFile;
	m_pBufPool = new BufferPool(Global::MAX_BUFFER_ROOMS_COUNT, Global::MAX_BUFFER_ROOMS_SIZE);
	m_pIndex = new HDLinkedListIndex(this, ConfigureMgr::get_enable_high_level_index());
	m_pExpireCache = new HDLinkedListExpireCache();

}

HDLinkedList::~HDLinkedList(void)
{
	if (m_pBufPool != NULL)
		delete m_pBufPool; 
	if (m_fsDataFile.is_open())
		m_fsDataFile.close();
	if (m_pIndex != NULL)
		delete m_pIndex;
	if (m_pExpireCache != NULL)
		delete m_pExpireCache;
}

HBOOL HDLinkedList::init()
{
	if (!m_bInitialized)
	{
		m_bInitialized = true;
		m_bValid = _init();
	}
	return m_bValid;
}

HBOOL HDLinkedList::_init()
{
	if (!Utils::file_exist(m_strDataFile))
	{
		Utils::open_file(m_fsDataFile, m_strDataFile);
		if (m_fsDataFile.is_open())
		{
			m_fsDataFile.write(FILE_TAG, strlen(FILE_TAG));
			m_llFileEndPos += strlen(FILE_TAG);

			m_fsDataFile.write((const HCHAR *)&Global::VERSION_CODE, sizeof(Global::VERSION_CODE));
			m_llFileEndPos += sizeof(Global::VERSION_CODE);

			m_fsDataFile.write((const HCHAR *)&m_ulLength, sizeof(m_ulLength));
			m_llFileEndPos += sizeof(m_ulLength);

			m_fsDataFile.write((const HCHAR *)&m_ulInvalidLength, sizeof(m_ulInvalidLength));
			m_llFileEndPos += sizeof(m_ulInvalidLength);

			m_fsDataFile.write((const HCHAR *)&m_llStartOffset, sizeof(m_llStartOffset));
			m_llFileEndPos += sizeof(m_llStartOffset);

			m_fsDataFile.flush();
			return true;
		}
	}
	else
	{
		Utils::open_file(m_fsDataFile, m_strDataFile);
		if (m_fsDataFile.is_open())
		{
			char file_tag[Global::DEFAULT_TEMP_BUFFER_SIZE] = { 0 };
			m_fsDataFile.read(file_tag, strlen(FILE_TAG));
			if (strcmp(file_tag, FILE_TAG) != 0)
			{
				m_fsDataFile.close();
				return false;
			}


			m_fsDataFile.read((HCHAR *)&m_ulVersionCode, sizeof(m_ulVersionCode));
			if (m_ulVersionCode > Global::VERSION_CODE)
			{
				m_fsDataFile.close();
				return false;
			}

			m_fsDataFile.read((HCHAR *)&m_ulLength, sizeof(m_ulLength));
			m_fsDataFile.read((HCHAR *)&m_ulInvalidLength, sizeof(m_ulInvalidLength));
			m_fsDataFile.read((HCHAR *)&m_llStartOffset, sizeof(m_llStartOffset));

			if (m_llStartOffset < (strlen(FILE_TAG)+sizeof(Global::VERSION_CODE)+sizeof(m_ulLength)+sizeof(m_ulInvalidLength)+sizeof(m_llStartOffset)))
			{
				m_fsDataFile.close();
				return false;
			}

			m_fsDataFile.seekp(0, ios::end);
			m_fsDataFile.seekg(0, ios::end);
			m_llFileEndPos = m_fsDataFile.tellg();

			init_indexs();
			return true;
		}
	}
	
	return false;
}

HBOOL HDLinkedList::is_valid()
{
	return m_bValid;
}

void HDLinkedList::init_indexs()
{
	if (m_fsDataFile.is_open())
	{
		if (m_ulLength > 0)
		{
			Logger::log_i("Loading Data Index: %s", m_strDataFile.c_str());

			HDLinkedListNode *pNode = NULL;
			HINT64 nextoffset = m_llStartOffset;

			while (nextoffset != 0)
			{
				GET_NODE(pNode, nextoffset);
				if (pNode != NULL)
				{
					if (pNode->get_expire_minutes() >= 0)
					{
						HINT32 ulExpiredMS = pNode->get_expire_minutes()*Global::SECONDS_IN_ONE_MINUTE;
						if (Utils::get_current_time_stamp() - pNode->get_time_stamp() > ulExpiredMS)
						{
							nextoffset = pNode->next();
							_erase(pNode);
							continue;
						}
						else
						{
							m_pExpireCache->post(pNode);
						}
					}

					m_pIndex->insert(pNode->key(), pNode->self(), true);
					nextoffset = pNode->next();
				}
				else
					break;
			}

			Logger::log_i("Loading Data Index Completed: %s", m_strDataFile.c_str());
		}
	}

	return;
}

void HDLinkedList::update_node(HDLinkedListNode &node)
{
	//大小不能改，只能更新前后指针值
	m_fsDataFile.seekg(node.self()+sizeof(HUINT32), ios::beg);
	m_fsDataFile.seekp(node.self()+sizeof(HUINT32), ios::beg);
	m_fsDataFile.write(node.buffer().c_str(), node.size());
	m_fsDataFile.flush();
	return;
}

void HDLinkedList::add_node(HDLinkedListNode &node)
{
	m_fsDataFile.seekg(node.self(), ios::beg);
	m_fsDataFile.seekp(node.self(), ios::beg);
	
	HUINT32 ulNodeLen = node.size();
	m_fsDataFile.write((const HCHAR*)&ulNodeLen, sizeof(ulNodeLen));
	m_fsDataFile.write(node.buffer().c_str(), node.size());

	m_ulLength++;
	m_fsDataFile.seekg(OFFSET_LENGTH_FIELD, ios::beg);
	m_fsDataFile.seekp(OFFSET_LENGTH_FIELD, ios::beg);
	m_fsDataFile.write((const HCHAR *)&m_ulLength, sizeof(m_ulLength));
	
	m_fsDataFile.flush();

	m_llFileEndPos += sizeof(ulNodeLen);
	m_llFileEndPos += node.size();

	m_pExpireCache->post(&node);

	return;
}

void HDLinkedList::update_length()
{
	m_fsDataFile.seekg(OFFSET_LENGTH_FIELD, ios::beg);
	m_fsDataFile.seekp(OFFSET_LENGTH_FIELD, ios::beg);
	m_fsDataFile.write((const HCHAR*)&m_ulLength, sizeof(m_ulLength));
	m_fsDataFile.flush();
	return;
}

void HDLinkedList::update_invalid_length()
{
	m_fsDataFile.seekg(OFFSET_INVALID_LENGTH_FIELD, ios::beg);
	m_fsDataFile.seekp(OFFSET_INVALID_LENGTH_FIELD, ios::beg);
	m_fsDataFile.write((const HCHAR*)&m_ulInvalidLength, sizeof(m_ulInvalidLength));
	m_fsDataFile.flush();
	return;
}

void HDLinkedList::update_start_offset()
{
	m_fsDataFile.seekg(OFFSET_STARTOFFSET_FIELD, ios::beg);
	m_fsDataFile.seekp(OFFSET_STARTOFFSET_FIELD, ios::beg);
	m_fsDataFile.write((const HCHAR*)&m_llStartOffset, sizeof(m_llStartOffset));
	m_fsDataFile.flush();
	return;
}

HBOOL HDLinkedList::put_when_length_eq_zero(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	m_pIndex->insert(k, m_llFileEndPos);

	m_llStartOffset = m_llFileEndPos;
	update_start_offset();	

	HDLinkedListNode node(k, v, ts, lExpireMinutes);
	node.set_self(m_llFileEndPos);
	add_node(node);

	return true;
}

HBOOL HDLinkedList::put_when_length_uneq_zero(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	HDLinkedListNode *pNode = NULL;

	HINT64 start = m_llStartOffset;
	HINT64 end = 0;
	HUINT32 level = Level_None;

	interval val = m_pIndex->get(k, m_llFileEndPos, level);
	if (level > Level_None)
	{
		start = val.llStartOffset;
		end = val.llEndOffset;
	}
	m_pIndex->insert(k, m_llFileEndPos);

	HINT64 nextoffset = start;
	while (nextoffset != 0)
	{
		GET_NODE(pNode, nextoffset);
		if (pNode != NULL)
		{
			if (level == m_pIndex->get_top_level())
			{
				HINT32 r = pNode->key().compare(k);
				if (r < 0)
				{
					nextoffset = pNode->next();
					if (nextoffset == 0)
					{
						HDLinkedListNode node(k, v, ts, lExpireMinutes);
						node.set_self(m_llFileEndPos);
						node.set_next(0);
						node.set_pre(pNode->self());
						add_node(node);

						pNode->set_next(node.self());
						update_node(*pNode);
						return true;
					}

				}
				else if (r == 0)//相等
				{
					if (pNode->value().compare(v) != 0)
						return false;
					else
						return true;
				}
				else
				{
					HDLinkedListNode node(k, v, ts, lExpireMinutes);
					node.set_self(m_llFileEndPos);
					node.set_next(pNode->self());
					node.set_pre(pNode->pre());
					add_node(node);

					if (pNode->self() == m_llStartOffset)
					{
						m_llStartOffset = node.self();
						update_start_offset();
					}

					HDLinkedListNode *pPreNode = NULL;
					GET_NODE(pPreNode, pNode->pre());
					if (pPreNode != NULL)
					{
						pPreNode->set_next(node.self());
						update_node(*pPreNode);
					}

					pNode->set_pre(node.self());
					update_node(*pNode);

					return true;
				}
			}
			else
			{
				HINT32 r = pNode->key().compare(k);
				if (r < 0)
				{
					HUINT64 llEndOffset = m_pIndex->query(pNode->key(), level+1).llEndOffset;
					if (llEndOffset != nextoffset)
					{
						HDLinkedListNode *pEndNode = NULL;
						GET_NODE(pEndNode, llEndOffset);
						if (pEndNode == NULL)
						{
							return false;
						}
						else
						{
							nextoffset = pEndNode->next();
							if (nextoffset == 0)
							{
								HDLinkedListNode node(k, v, ts, lExpireMinutes);
								node.set_self(m_llFileEndPos);
								node.set_next(0);
								node.set_pre(pEndNode->self());
								add_node(node);

								pEndNode->set_next(node.self());
								update_node(*pEndNode);
								return true;
							}
						}
					}
					else
					{
						nextoffset = pNode->next();
						if (nextoffset == 0)
						{
							HDLinkedListNode node(k, v, ts, lExpireMinutes);
							node.set_self(m_llFileEndPos);
							node.set_next(0);
							node.set_pre(pNode->self());
							add_node(node);

							pNode->set_next(node.self());
							update_node(*pNode);
							return true;
						}

					}
				}
				else if (r > 0)
				{
					HDLinkedListNode node(k, v, ts, lExpireMinutes);
					node.set_self(m_llFileEndPos);
					node.set_next(pNode->self());
					node.set_pre(pNode->pre());
					add_node(node);

					if (pNode->self() == m_llStartOffset)
					{
						m_llStartOffset = node.self();
						update_start_offset();
					}

					HDLinkedListNode *pPreNode = NULL;
					GET_NODE(pPreNode, pNode->pre());
					if (pPreNode != NULL)
					{
						pPreNode->set_next(node.self());
						update_node(*pPreNode);
					}

					pNode->set_pre(node.self());
					update_node(*pNode);

					return true;
				}
				else
				{
					Logger::log_w("Find invaild index when put a key [%s]!!!", k.c_str());
					return false;
				}
			}
		}
		else
			break;
	}

	return false;
}

HBOOL HDLinkedList::_put(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	if (!m_fsDataFile.is_open()) 
		return false;
	if (m_ulLength == 0)
		return put_when_length_eq_zero(k, v, ts, lExpireMinutes);
	else
		return put_when_length_uneq_zero(k, v, ts, lExpireMinutes);
}

HBOOL HDLinkedList::_erase(const string &k)
{
	if (!m_fsDataFile.is_open()) 
		return false;
	HDLinkedListNode node = get_node_by_key(k);
	if (!node.is_valid())
		return false;
	return _erase(&node);
}

HBOOL HDLinkedList::_erase(HDLinkedListNode *pNode)
{
	if (pNode != NULL)
	{	
		m_pIndex->erase(pNode);

		if (1)
		{
			HDLinkedListNode *pPreNode = NULL;
			GET_NODE(pPreNode, pNode->pre());
			if (pPreNode != NULL)
			{
				pPreNode->set_next(pNode->next());
				update_node(*pPreNode);
			}
		}

		if (1)
		{
			HDLinkedListNode *pNextNode = NULL;
			GET_NODE(pNextNode, pNode->next());
			if (pNextNode != NULL)
			{
				pNextNode->set_pre(pNode->pre());
				update_node(*pNextNode);
			}
		}

		if (pNode->self() == m_llStartOffset && pNode->next() != 0)
		{
			m_llStartOffset = pNode->next();
			update_start_offset();
		}

		m_ulLength--;
		update_length();
		m_ulInvalidLength++;
		update_invalid_length();

		pNode->set_deleted(true);
		update_node(*pNode);

		return true;
	}
	else
		return false;
}

HDLinkedListNode HDLinkedList::get_node_by_key(const string &k)
{
	HINT64 start = 0;
	HINT64 end = 0;

	interval val = m_pIndex->query(k, m_pIndex->get_top_level());
	if (val.llStartOffset > 0 && val.llEndOffset > 0)
	{
		start = val.llStartOffset;
		end = val.llEndOffset;
	}
	else
	{
		return HDLinkedListNode(NULL, 0);
	}

	HDLinkedListNode *pNode = NULL;
	HINT64 nextoffset = start;
	while (nextoffset != 0)
	{
		GET_NODE(pNode, nextoffset);
		if (pNode != NULL)
		{
			if (!pNode->is_deleted() && pNode->key().compare(k) == 0)//找到
			{
				return *pNode;
			}
			else if (pNode->next() == 0)
			{
				return HDLinkedListNode(NULL, 0);
			}
			else
			{
				if (pNode->self() == end)
				{
					return HDLinkedListNode(NULL, 0);
				}
				else
				{
					nextoffset = pNode->next();
				}
			}
		}
		else
			break;
	}

	return HDLinkedListNode(NULL, 0);
}

HBOOL HDLinkedList::_get(const string &k, string &v, HINT32 &lExpireMinutes, HDLinkedListNode *p)
{
	if (!m_fsDataFile.is_open()) 
		return false;

	HINT64 start = 0;
	HINT64 end = 0;
	interval val = m_pIndex->query(k, m_pIndex->get_top_level());
	if (val.llStartOffset > 0 && val.llEndOffset > 0)
	{
		start = val.llStartOffset;
		end = val.llEndOffset;
	}
	else
	{
		return false;
	}

	//Logger::log_i("key=%s, start=%d, end=%d",k.c_str(), start, end);

	HDLinkedListNode *pNode = NULL;
	HINT64 nextoffset = start;
	while (nextoffset != 0)
	{
		GET_NODE(pNode, nextoffset);
		if (pNode != NULL)
		{
			if (!pNode->is_deleted() && pNode->key().compare(k) == 0)//找到
			{
				v = pNode->value();
				lExpireMinutes = pNode->get_expire_minutes();
				if (p != NULL)
					*p = *pNode;
				return true;
			}
			else if (pNode->next() == 0)
			{
				return false;
			}
			else
			{
				if (pNode->self() == end)
					return false;
				else
					nextoffset = pNode->next();
			}	
		}
		else
			break;
	}

	return false;
}

HBOOL HDLinkedList::exists(const string &k)
{
	string v;
	HINT32 lExpireMinutes = -1;
	return get(k, v, lExpireMinutes);
}

HUINT32 HDLinkedList::length()
{
	return m_ulLength;
}

HUINT32 HDLinkedList::version()
{
	return m_ulVersionCode;
}

HDLinkedListNode HDLinkedList::get_node(HINT64 offset)
{
	if (offset <= 0) 
		return HDLinkedListNode(NULL, 0);

	HINT32 iBufferPoolIndex = -1;
	if (m_pBufPool != NULL)
		iBufferPoolIndex = (*m_pBufPool).lease();
	if (iBufferPoolIndex == -1)
		return HDLinkedListNode(NULL, 0);

	HUINT32 ulNodeLen = 0;
	m_SeekFileLock.lock();
	m_fsDataFile.seekg(offset, ios::beg);
	m_fsDataFile.seekp(offset, ios::beg);
	m_fsDataFile.read((HCHAR *)&ulNodeLen, sizeof(ulNodeLen));
	if (ulNodeLen > HDLinkedListNode::min_size() && ulNodeLen <= HDLinkedListNode::max_size())
		m_fsDataFile.read((HCHAR *)(*m_pBufPool).get(iBufferPoolIndex), ulNodeLen);
	else
		ulNodeLen = 0;
	m_SeekFileLock.unlock();

	HDLinkedListNode node((HCHAR *)(*m_pBufPool).get(iBufferPoolIndex), ulNodeLen);
	(*m_pBufPool).ret(iBufferPoolIndex);

	if (!node.is_valid())
		Logger::log_w("A invalid key in the data file : %s!", m_strDataFile.c_str());
	return node;
}

void HDLinkedList::_output_to_map(map<string, KeyValueInfo> &mapOutput, HUINT32 limit)
{
	if (m_fsDataFile.is_open())
	{
		if (m_ulLength > 0)
		{
			HUINT32 count = 0;
			HINT64 nextoffset = m_llStartOffset;
			HDLinkedListNode *pNode = NULL;
			while (nextoffset != 0 && count < limit)
			{
				GET_NODE(pNode, nextoffset);
				if (pNode != NULL)
				{
					count++;
					KeyValueInfo kv(pNode->key(), pNode->value(), pNode->get_expire_minutes(), pNode->get_time_stamp());
					mapOutput[pNode->key()] = kv;
					nextoffset = pNode->next();
				}
				else
					break;
			}
		}
	}

	return;
}

HUINT32 HDLinkedList::invalid_length()
{
	return m_ulInvalidLength;
}

HBOOL HDLinkedList::_trim_for_erase()
{
	if (!m_fsDataFile.is_open()) 
		return false;

	Logger::log_i("Trimming Data File: %s", m_strDataFile.c_str());

	string strTempDataFile = m_strDataFile + ".temp";
	fstream fs;
	Utils::open_file(fs, strTempDataFile);
	if (fs.is_open())
	{
		m_pIndex->clear();
		m_llFileEndPos = 0;
		m_ulInvalidLength = 0;

		fs.write(FILE_TAG, strlen(FILE_TAG));
		m_llFileEndPos += strlen(FILE_TAG);

		fs.write((const HCHAR *)&Global::VERSION_CODE, sizeof(Global::VERSION_CODE));
		m_llFileEndPos += sizeof(Global::VERSION_CODE);

		fs.write((const HCHAR *)&m_ulLength, sizeof(m_ulLength));
		m_llFileEndPos += sizeof(m_ulLength);

		fs.write((const HCHAR *)&m_ulInvalidLength, sizeof(m_ulInvalidLength));
		m_llFileEndPos += sizeof(m_ulInvalidLength);
		HINT64 llTempStartOffset = m_llFileEndPos+sizeof(HUINT32);

		fs.write((const HCHAR *)&llTempStartOffset, sizeof(llTempStartOffset));
		m_llFileEndPos += sizeof(llTempStartOffset);

		if (!m_fsDataFile.is_open())
		{
			if (m_ulLength > 0)
			{
				HINT64 nextoffset = m_llStartOffset;
				HDLinkedListNode *pNode = NULL;
				HDLinkedListNode *pPreNode = NULL;
				while (nextoffset != 0)
				{
					GET_NODE(pNode, nextoffset);
					if (pNode != NULL)
					{
						nextoffset = pNode->next();
						pNode->set_self(m_llFileEndPos);
						if (pNode->next() != 0)
							pNode->set_next(m_llFileEndPos + pNode->size());
						if (pNode->pre() != 0 && pPreNode != NULL)
							pNode->set_pre(m_llFileEndPos - pPreNode->size());

						HUINT32 ulNodeLen = pNode->size();
						fs.write((const HCHAR *)&ulNodeLen, sizeof(ulNodeLen));
						m_llFileEndPos += sizeof(ulNodeLen);

						fs.write(pNode->buffer().c_str(), pNode->size());
						m_llFileEndPos += pNode->size();
						fs.flush();

						m_pIndex->insert(pNode->key(), pNode->self(), true);

						pPreNode = pNode;
					}
					else
						break;
				}
			}
		}
		fs.close();
		m_fsDataFile.close();
		m_llStartOffset = llTempStartOffset;
			
		Utils::delete_file(m_strDataFile);
		Utils::rename_file(strTempDataFile, m_strDataFile);

		Utils::open_file(m_fsDataFile, m_strDataFile);
	}

	Logger::log_i("Trimming Data File Completed: %s", m_strDataFile.c_str());

	return true;
}

HUINT32 HDLinkedList::trim_for_expire()
{
	HUINT32 ret = 0;
	m_RWLock.lock(RWLock::LOCK_LEVEL_WRITE);
	ret = _trim_for_expire();
	m_RWLock.unlock();
	return ret;
}

HBOOL HDLinkedList::load_expire_cache()
{
	HBOOL ret = false;
	m_RWLock.lock(RWLock::LOCK_LEVEL_READ);
	ret = _load_expire_cache();
	m_RWLock.unlock();
	return ret;
}

HBOOL HDLinkedList::_load_expire_cache()
{
	if (!m_fsDataFile.is_open()) 
		return false;

	if (m_ulLength > 0)
	{	
		m_pExpireCache->clear();
		HINT64 nextoffset = m_llStartOffset;
		HDLinkedListNode *pNode = NULL;
		while (nextoffset != 0)
		{
		 	GET_NODE(pNode, nextoffset);
		 	if (pNode != NULL)
		 	{
		 		nextoffset = pNode->next();
		 		if (pNode->get_expire_minutes() < 0)//小于0代表永久不过期
		 			continue;
				m_pExpireCache->post(pNode);
		 	}
		 	else
		 		break;
		}
	}

	return true;

}

HUINT32 HDLinkedList::_trim_for_expire()
{
	if (!m_fsDataFile.is_open()) 
		return false;

	HUINT32 ulCount = 0; 
	if (m_ulLength > 0)
	{
		vector<vector<HUINT32>> vec = m_pExpireCache->peek();
		for (HUINT32 i=0; i<vec.size(); i++)
		{
			vector<HUINT32> vecNode = vec[i];
			for (HUINT32 j=0; j<vecNode.size(); j++)
			{
				HDLinkedListNode *pNode = NULL;
				GET_NODE(pNode, vecNode[j]);
				if (pNode != NULL && !pNode->is_deleted())
				{
					_erase(pNode);
					ulCount++;
				}
			}
		}
	}
	
	return ulCount;
}

HBOOL HDLinkedList::update(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	HBOOL ret = false;
	m_RWLock.lock(RWLock::LOCK_LEVEL_WRITE);

	HDLinkedListNode node;
	string val;
	HINT32 et = 0;
	ret = _get(k, val, et, &node);
	if (ret)
	{
		if (val.compare(v) != 0 || lExpireMinutes != 0)
		{
			ret = _erase(&node);
			if (ret)
			{
				if (lExpireMinutes != 0)
					ret = _put(k, v, ts, lExpireMinutes);
				else
					ret = _put(k, v, node.get_time_stamp(), node.get_expire_minutes());
			}
		}
	}

	m_RWLock.unlock();
	return ret;
}

HBOOL HDLinkedList::put(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes)
{
	HBOOL ret = false;
	m_RWLock.lock(RWLock::LOCK_LEVEL_WRITE);
	ret = _put(k, v, ts, lExpireMinutes);
	m_RWLock.unlock();
	return ret;
}

HBOOL HDLinkedList::erase(const string &k)
{
	HBOOL ret = false;
	m_RWLock.lock(RWLock::LOCK_LEVEL_WRITE);
	ret = _erase(k);
	if (invalid_length() > Global::MAX_INVALID_KEYS)
		_trim_for_erase();
	m_RWLock.unlock();
	return ret;
}

HBOOL HDLinkedList::get(const string &k, string &v, HINT32 &lExpireMinutes)
{
	HBOOL ret = false;
	m_RWLock.lock(RWLock::LOCK_LEVEL_READ);
	ret = _get(k, v, lExpireMinutes);
	m_RWLock.unlock();
	return ret;
}

void HDLinkedList::output_to_map(map<string, KeyValueInfo> &m, HUINT32 limit)
{
	m_RWLock.lock(RWLock::LOCK_LEVEL_READ);
	_output_to_map(m, limit);
	m_RWLock.unlock();
	return;
}

HBOOL HDLinkedList::trim_for_erase()
{
	HBOOL ret = false;
	m_RWLock.lock(RWLock::LOCK_LEVEL_WRITE);
	ret = _trim_for_erase();
	m_RWLock.unlock();
	return ret;
}

vector<HUINT32> HDLinkedList::get_indexs_size()
{
	return m_pIndex->size();
}

HUINT32 HDLinkedList::get_expire_cache_size()
{
	return m_pExpireCache->size();
}