#include "HDSMClientImpl.h"
#include <iostream>
#include <string.h>
using namespace std;

#define CHECK_SN(_sn_) if (_sn_ > 1000000) _sn_ = 2;

HDSMClientImpl::HDSMClientImpl(void)
{
	BaseSocket::init();
	m_bValid = false;
	m_ulCurSN = 1;
}

HDSMClientImpl::~HDSMClientImpl(void)
{
	BaseSocket::clean();
}

Result HDSMClientImpl::echo()
{
	CHECK_SN(m_ulCurSN);

	Result result;
	Task task(OP_ECHO, m_ulCurSN++);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::get(const string &k)
{
	CHECK_SN(m_ulCurSN);

	Result result(k);
	if (k.size() > m_ulMaxKeyLen)
		return result;

	Task task(OP_GET, m_ulCurSN++, k);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::update(const string &k, const string &v, HINT32 lExpireMinutes)
{
	CHECK_SN(m_ulCurSN);

	Result result(k);
	if (k.size() > m_ulMaxKeyLen || v.size() > m_ulMaxValueLen)
		return result;

	Task task(OP_UPDATE, m_ulCurSN++, k, v, lExpireMinutes);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::put(const string &k, const string &v, HINT32 lExpireMinutes)
{
	CHECK_SN(m_ulCurSN);

	Result result(k, v);
	if (k.size() > m_ulMaxKeyLen || v.size() > m_ulMaxValueLen)
		return result;

	Task task(OP_PUT, m_ulCurSN++, k, v, lExpireMinutes);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::erase(const string &k)
{
	CHECK_SN(m_ulCurSN);

	Result result(k);
	if (k.size() > m_ulMaxKeyLen)
		return result;

	Task task(OP_ERASE, m_ulCurSN++, k);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::exists(const string &k)
{
	CHECK_SN(m_ulCurSN);

	Result result(k);
	if (k.size() > m_ulMaxKeyLen)
		return result;

	Task task(OP_EXISTS, m_ulCurSN++, k);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::length()
{
	CHECK_SN(m_ulCurSN);

	Result result;
	Task task(OP_LENGTH, m_ulCurSN++);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::quit()
{
	CHECK_SN(m_ulCurSN);

	Result result;
	Task task(OP_QUIT, m_ulCurSN++);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::keys(HUINT32 limit)
{
	CHECK_SN(m_ulCurSN);
	
	Result result;
	Task task(OP_KEYS, m_ulCurSN++, limit);
	do_task(task, result);
	return result;
}

Result HDSMClientImpl::shutdown(const string &password)
{
	CHECK_SN(m_ulCurSN);

	Result result;
	Task task(OP_SHUTDOWN, m_ulCurSN++, password);
	do_task(task, result);
	return result;
}

HBOOL HDSMClientImpl::initialize()
{
	if (!m_sock.is_valid())
		m_sock.create(AF_INET, SOCK_STREAM, 0);

	if (m_sock.connect(m_strSrvIP.c_str(), m_usSrvPort) == SOCKET_ERROR)
	{
		//HINT32 err = BaseSocket::get_error();
		m_sock.close();
		return false;
	}

	Result r = echo();
	if (r.is_valid())
	{
		m_ulMaxKeyLen = r.get_max_key_length();
		m_ulMaxValueLen = r.get_max_value_length();
		m_bValid = true;
		return true;
	}
	else
	{
		unintialize();
		return false;
	}
}

void HDSMClientImpl::unintialize()
{
	m_bValid = false;
	if (!m_sock.is_valid())
		m_sock.close();
	return;
}

HBOOL HDSMClientImpl::do_task(Task &task, Result &result)
{
	if (!m_sock.is_valid())
		initialize();

	if (m_sock.is_valid())
	{
		HINT32 nTemp = 0;
		HUINT32 nSent = 0;
		string buffer = task.buffer();
		const HCHAR *pBuf = buffer.c_str();
		while ((nTemp = m_sock.send(pBuf+nSent, buffer.size()-nSent, 0)) != SOCKET_ERROR)
		{
			nSent += nTemp;
			if (nSent >= buffer.size())
				break;
		}

		if (nTemp == SOCKET_ERROR)
		{
			cout<< "send error!" << endl;
			unintialize();
			return false;
		}

		if (m_sock.wait_timeout(m_iTimeout*1000))
		{
			unintialize();
			return false;
		}

		do 
		{
			HCHAR szRecvBuffer[1024] = {0};
			HUINT32 nTotalRecvLen = 0;
			HINT32 nRecvLen = 0;
			HINT32 nWillRecvLen = 1024;
			
			result.reset();

			HUINT32 ulDataLen = 0;
			m_sock.recv((HCHAR *)&ulDataLen, sizeof(HUINT32), 0);

			string strRecv;
			while (0 != (nRecvLen = m_sock.recv(szRecvBuffer, nWillRecvLen, 0)))
			{
				if (nRecvLen == SOCKET_ERROR)
				{
					cout<< "recv error!" << endl;
					unintialize();
					return false;
				}

				strRecv.append(szRecvBuffer, nRecvLen);
				nTotalRecvLen += nRecvLen;
				if (nTotalRecvLen == ulDataLen)
					break;

				memset(szRecvBuffer, 0, 1024);

				if (ulDataLen - nTotalRecvLen < nWillRecvLen)
					nWillRecvLen = ulDataLen - nTotalRecvLen;
			}

			result.load(strRecv.c_str(), ulDataLen);
		}while (result.get_SN() != task.get_SN());

		return result.is_valid();
	}

	return false;
}

HUINT32 HDSMClientImpl::get_sock()
{
	return (SOCKET)m_sock;
}

HBOOL HDSMClientImpl::is_valid()
{
	return m_bValid;
}

