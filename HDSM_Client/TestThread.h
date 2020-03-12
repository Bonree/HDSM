#pragma once
#include "BaseThread.h"
#include "HDSMClient.h"
#include <string>
using namespace std;

class TestThread :
	public BaseThread
{
public:
	TestThread(HDSMClient *pClient, HUINT32 op, HINT32 timeout = 5000);
	~TestThread(void);
public:
	virtual void run();
private:
	void put_many_keys();
	void get_many_keys();
	void erase_many_keys();
public:
	void set_key_offset_interval(HINT32 nStartOffset, HINT32 nEndOffset);
private:
	HDSMClient *m_pClient;
	HINT32 m_nStartOffset, m_nEndOffset;
	HUINT32 m_ulTestOpType;
};

