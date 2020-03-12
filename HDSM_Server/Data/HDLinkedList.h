#pragma once
#include <string>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include "BufferPool.h"
#include "HDLinkedListNode.h"
#include "HDLinkedListIndex.h"
#include "IHDLinkedList.h"
#include "HDLinkedListExpireCache.h"
#include "RWLock.h"
#include "HDSMOperateType.h"
#include "TypeDefine.h"
using namespace std;

class HDLinkedList 
	: public IHDLinkedList
{
public:
	HDLinkedList(const string &strDataFile);
	virtual ~HDLinkedList(void);
public:
	HBOOL put(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes);
	HBOOL update(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes);//lExpireMinutes=0表示不更新过期时间
	HBOOL erase(const string &k);
	HBOOL get(const string &k, string &v, HINT32 &lExpireMinutes);
	HBOOL exists(const string &k);
	HUINT32 length();
	HUINT32 version();
	void output_to_map(map<string, KeyValueInfo> &mapOutput, HUINT32 limit = 100);
	HBOOL trim_for_erase();
	HUINT32 trim_for_expire();
	HBOOL load_expire_cache();
	HBOOL is_valid();
	HUINT32 invalid_length();
	HBOOL init();
	virtual HDLinkedListNode get_node(HINT64 offset);
	HUINT32 get_expire_cache_size();
public:
	vector<HUINT32> get_indexs_size();
private:
	HBOOL _put(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes);
	HBOOL _erase(const string &k);
	HBOOL _erase(HDLinkedListNode *pNode);
	HBOOL _get(const string &k, string &v, HINT32 &lExpireMinutes, HDLinkedListNode *p = NULL);
	void _output_to_map(map<string, KeyValueInfo> &m, HUINT32 limit);
	HBOOL _trim_for_erase();
	HUINT32 _trim_for_expire();
	HBOOL _load_expire_cache();
private:
	HDLinkedListNode get_node_by_key(const string &k);
	HBOOL _init();
private:
	HBOOL put_when_length_eq_zero(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes);
	HBOOL put_when_length_uneq_zero(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes);
	void add_node(HDLinkedListNode &node);
	void update_node(HDLinkedListNode &node);
	void update_start_offset();
	void update_length();
	void update_invalid_length();
	void init_indexs();
private:
	BufferPool *m_pBufPool;
	string		m_strDataFile;
	HINT64		m_llStartOffset;//-1标识无效
	HINT64		m_llFileEndPos;
	fstream		m_fsDataFile;
	HUINT32		m_ulVersionCode;
	HUINT32		m_ulLength;
	HUINT32		m_ulInvalidLength;
	RWLock		m_RWLock;
	HBOOL		m_bValid;
	HBOOL		m_bInitialized;
private:
	SimpleLock	m_SeekFileLock;
private:
	HDLinkedListIndex		*m_pIndex;
	HDLinkedListExpireCache *m_pExpireCache;
public:
	const HCHAR *FILE_TAG;
	const HINT32 OFFSET_LENGTH_FIELD;
	const HINT32 OFFSET_INVALID_LENGTH_FIELD;
	const HINT32 OFFSET_STARTOFFSET_FIELD;
};

