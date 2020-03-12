#pragma once
#include <string>
#include "TypeDefine.h"
using namespace std;

class HDLinkedListNode
{
public:
	HDLinkedListNode();
	HDLinkedListNode(const HDLinkedListNode &node);
	HDLinkedListNode(const string &k, const string &v, HUINT64 ts, HINT32 lExpireMinutes);
	HDLinkedListNode(const HCHAR *pszBuf, HUINT32 nBufLen);
	~HDLinkedListNode(void);
public:
	string buffer();
	void set_deleted(HBOOL deleted);
	HBOOL is_deleted();
	HBOOL is_valid();
	HUINT32 size();
	static HUINT32 max_size();
	static HUINT32 min_size();
	void set_self(HINT64 self);
	void set_next(HINT64 next);
	void set_pre(HINT64 pre);
	void set_value(const string &v);
public:
	const string &key();
	const string &value();
	HINT64 next() const;
	HINT64 pre() const;
	HINT64 self() const;
	HUINT64 get_time_stamp() const;
	HINT32 get_expire_minutes() const;
private:
	HUINT8	m_ucFlag;
	HINT64	m_llNextPtr;
	HINT64	m_llPrePtr;
	HINT64	m_llSelfPtr;
	string	m_strKey;
	string	m_strValue;
	HBOOL	m_bValid;
	HUINT64 m_ullTimestamp;
	HINT32	m_lExpireMinutes;
};

