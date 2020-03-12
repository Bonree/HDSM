#pragma once
#include <map>
#include <vector>
#include "HDLinkedListNode.h"
#include "TypeDefine.h"

using namespace std;

class HDLinkedListExpireCache
{
public:
	HDLinkedListExpireCache(void);
	virtual ~HDLinkedListExpireCache(void);

public:
	HBOOL post(const HDLinkedListNode *pNode);
	void clear();
	vector<vector<HUINT32>> peek();
	HUINT32 size();
private:
	//默认只加载最近24小时将要过期的key偏移信息
	//需要处理的地方包括初始化、写操作、清理已删除数据
	//默认凌晨3点开始重新加载
	map<HUINT64, vector<HUINT32>> m_NearExpiredKeys;
};

