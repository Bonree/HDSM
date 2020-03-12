#ifdef WIN32
#include <WinSock2.h>  
#endif
#include "Connection.h"
#include "Utils.h"
#include <string.h>
#include "../Logger.h"
#include "../Global.h"

Connection::Connection()
{
	m_bCloseWait = false;
	m_ulReadOffset = 0;
	memset(m_szReadBuffer, 0, Global::DEFAULT_TEMP_BUFFER_SIZE);
}

Connection::~Connection(void)
{
	m_lstReponses.empty();
}

void Connection::push_response(Response r)
{
	m_lock.lock();
	m_lstReponses.push(r);
	m_lock.unlock();
	return;
}

void Connection::pop_response()
{
	m_lock.lock();
	m_lstReponses.pop();
	m_lock.unlock();
	return;
}

Connection::Response *Connection::get_front_response()
{
	Response *pr = NULL;
	m_lock.lock();
	if (m_lstReponses.size() > 0)
		pr = &m_lstReponses.front();
	m_lock.unlock();
	return pr;
}

HUINT32 Connection::get_response_count()
{
	return m_lstReponses.size();
}

BaseSocket& Connection::get_sock()
{
	return m_sock;
}

void Connection::set_sock(BaseSocket &s)
{
	m_sock = s;
}

void Connection::set_index(HUINT32 index)
{
	m_nIndexInConnectionPool = index;
}

void Connection::disabled()
{
	m_lock.lock();
	m_lstReponses.empty();
	m_bCloseWait = false;
	m_sock.close();
	m_ulReadOffset = 0;
	memset(m_szReadBuffer, 0, Global::DEFAULT_TEMP_BUFFER_SIZE);
	m_lock.unlock();
	return;
}

HUINT32 Connection::get_index()
{
	return m_nIndexInConnectionPool;
}

void Connection::wait_for_send_respond_right()
{
	m_SendRespondRight.lock();
}

void Connection::ret_send_respond_right()
{
	m_SendRespondRight.unlock();
}

void Connection::set_close_wait()
{
	Logger::log_d("the Connection [sock=%d] is closing!", m_sock.get_raw_socket());
	m_bCloseWait = true;
}

HBOOL Connection::is_close_wait()
{
	return m_bCloseWait;
}

string Connection::get_peer_ip()
{
	HCHAR szIP[Global::DEFAULT_TEMP_BUFFER_SIZE] = {0};
	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	if (m_sock.is_valid())
	{
		if (!getpeername((SOCKET)m_sock, (struct sockaddr *)&sa, &len))
			sprintf(szIP, "%s:%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
	}
	return szIP;
}