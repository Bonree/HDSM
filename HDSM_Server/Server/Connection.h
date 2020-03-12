#pragma once
#include <string>
#include <queue>
#include "SimpleLock.h"
#include "BaseSocket.h"
#include "../Global.h"
#include "TypeDefine.h"

using namespace std;

class Connection
{
public:
	Connection();
	~Connection(void);
public:
	typedef struct tagResponse
	{
		HUINT32 operate;
		string buffer;
		HUINT32 ulWrittenOffset;
		tagResponse(string b, HUINT32 op){buffer = b;ulWrittenOffset = 0;operate = op;}
	}Response;
public:
	void push_response(Response r);
	Response *get_front_response();
	void pop_response();
	HUINT32 get_response_count();
public:
	BaseSocket& get_sock();
	string get_peer_ip();
	HUINT32 get_index();
	void set_sock(BaseSocket &s);
	void set_index(HUINT32 index);
	HBOOL is_close_wait();
	void set_close_wait();
	void disabled();
public:
	void wait_for_send_respond_right();
	void ret_send_respond_right();
public:
	HCHAR			m_szReadBuffer[Global::DEFAULT_TEMP_BUFFER_SIZE];
	HUINT32	m_ulReadOffset;
private:
	BaseSocket		m_sock;
	queue<Response> m_lstReponses;
	SimpleLock		m_lock;
	HUINT32			m_nIndexInConnectionPool;
	HBOOL			m_bCloseWait;
private:
	SimpleLock		m_SendRespondRight;
};

