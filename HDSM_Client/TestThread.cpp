#include "TestThread.h"
#include <iostream>

TestThread::TestThread(HDSMClient *pClient, HUINT32 op, HINT32 timeout/* = 5000*/)
{
	m_pClient = pClient;
	m_ulTestOpType = op;
}

TestThread::~TestThread(void)
{
}

void TestThread::set_key_offset_interval(HINT32 nStartOffset, HINT32 nEndOffset)
{
	m_nStartOffset = nStartOffset;
	m_nEndOffset = nEndOffset;
}

void TestThread::get_many_keys()
{
	Result r;
	HINT32 failed = 0;
	HINT32 missed = 0;
	HUINT32 ulTotalTime = 0;
	HUINT32 ulToatlKeys = 0;
	for (HUINT64 i=m_nStartOffset; i<m_nEndOffset; i++)
	{
		string key = "k@abcdefg#" + std::to_string(i);
		string value = "v@abcdefg#" + std::to_string(i);

		r.reset();
		r = m_pClient->get(key);
		ulTotalTime += r.get_total_time();
		if (r.is_valid() && value.compare(r.get_value()) != 0)
		{
			missed++;
			cout<<"GET: ("<< key.c_str() << ", " << r.get_value().c_str() << ")" << endl;
		}
		if (!r.is_valid() || !r.get_ret())
			failed++;

		if ((i!=m_nStartOffset) && (i-m_nStartOffset)%1000 == 0)
		{
			ulToatlKeys += 1000;
			cout << "GET " << ulToatlKeys << " keys!AvgTime£º"<< ulTotalTime/10000.0 << " ms" << endl;
			ulTotalTime = 0;
		}
	}

	cout << "Total GET " << (m_nEndOffset-m_nStartOffset) 
		<< " keys, Successed :"<< m_nEndOffset-m_nStartOffset-failed 
		<<", Failed : " << failed << ", Missed: " << missed << endl;
	return;
}

void TestThread::put_many_keys()
{
	Result r;
	HINT32 failed = 0;
	HINT32 k = 0;
	HUINT32 ulTotalTime = 0;
	for (HUINT64 i=m_nStartOffset; i<m_nEndOffset; i++)
	{
		string key = "k@abcdefg#" + std::to_string(i);
		string value = "v@abcdefg#" + std::to_string(i);
		HINT32 lExpireMinutes = -1; 
		r.reset();
		r = m_pClient->put(key, value, lExpireMinutes);

		ulTotalTime += r.get_total_time();
		
		if (!r.is_valid() || !r.get_ret())
			failed++;

		k++;

		if (k % 1000 == 0)
		{
			cout << "AvgTime£º"<< ulTotalTime/1000.0 << " ms" << endl;
			ulTotalTime = 0;
			r.reset();
			r = m_pClient->length();
			if (r.is_valid())
			{
			 	if (r.get_ret())
			 		cout<<"===Total Length: "<< r.get_length() << "==="<< endl;
			 	else
			 		cout<< "length failed!!"<<endl;
			}
		}

	}
	cout << "Total PUT " << (m_nEndOffset-m_nStartOffset) 
		<< " keys, Successed: " << m_nEndOffset-m_nStartOffset-failed 
		<< ", Failed: " << failed << endl; 
	return;
}

void TestThread::erase_many_keys()
{
	Result r;
	HINT32 failed = 0;
	HINT32 k = 0;
	HUINT32 ulTotalTime = 0;
	for (HUINT64 i=m_nStartOffset; i<m_nEndOffset; i++)
	{
		string key = "k@abcdefg#" + std::to_string(i);
		r.reset();
		r = m_pClient->erase(key);

		ulTotalTime += r.get_total_time();

		if (!r.is_valid() || !r.get_ret())
			failed++;

		k++;

		if (k % 1000 == 0)
		{
			cout << "AvgTime£º"<< ulTotalTime/10000.0 << " ms" << endl;
			ulTotalTime = 0;
			r.reset();
			r = m_pClient->length();
			if (r.is_valid())
			{
				if (r.get_ret())
					cout<<"===Total Length: "<< r.get_length() << "==="<< endl;
				else
					cout<< "length failed!!"<<endl;
			}
		}

	}
	cout << "Total ERASE " << (m_nEndOffset-m_nEndOffset) 
		<< " keys, Successed: " << m_nEndOffset-m_nStartOffset-failed 
		<<", Failed: " << failed << endl; 
	return;
}

void TestThread::run()
{
	if (m_pClient && m_pClient->is_valid())
	{
		if (m_ulTestOpType == OP_PUT)
			put_many_keys();
		else if (m_ulTestOpType == OP_GET)
			get_many_keys();
		else if (m_ulTestOpType == OP_ERASE)
			erase_many_keys();
		else
			return;
	}
	
	return;
}