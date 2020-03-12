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
	//Ĭ��ֻ�������24Сʱ��Ҫ���ڵ�keyƫ����Ϣ
	//��Ҫ����ĵط�������ʼ����д������������ɾ������
	//Ĭ���賿3�㿪ʼ���¼���
	map<HUINT64, vector<HUINT32>> m_NearExpiredKeys;
};

