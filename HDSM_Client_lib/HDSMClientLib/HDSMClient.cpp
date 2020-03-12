#include "HDSMClient.h"
#include <iostream>
#include "HDSMClientImpl.h"

HDSMClient::HDSMClient(const string &strSrvIP, HUINT16 usSrvPort, HINT32 timeout/* = 5000*/)
{
	Impl = (void *)new HDSMClientImpl;
	((HDSMClientImpl *)Impl)->m_strSrvIP = strSrvIP;
	((HDSMClientImpl *)Impl)->m_usSrvPort = usSrvPort;
	((HDSMClientImpl *)Impl)->m_iTimeout = timeout;
	((HDSMClientImpl *)Impl)->initialize();
}

HDSMClient::~HDSMClient(void)
{
	((HDSMClientImpl *)Impl)->unintialize();
	delete ((HDSMClientImpl *)Impl);
}

Result HDSMClient::quit()
{
	Result r = ((HDSMClientImpl *)Impl)->quit();
	((HDSMClientImpl *)Impl)->unintialize();
	return r;
}

HUINT32 HDSMClient::get_max_key_len()
{
	return ((HDSMClientImpl *)Impl)->m_ulMaxKeyLen;
}

HUINT32 HDSMClient::get_max_value_len()
{
	return ((HDSMClientImpl *)Impl)->m_ulMaxValueLen;
}

Result HDSMClient::get(const string &k)
{
	return ((HDSMClientImpl *)Impl)->get(k);
}

Result HDSMClient::update(const string &k, const string &v, HINT32 lExpireMinutes)
{
	return ((HDSMClientImpl *)Impl)->update(k, v, lExpireMinutes);
}

Result HDSMClient::put(const string &k, const string &v, HINT32 lExpireMinutes)
{
	return ((HDSMClientImpl *)Impl)->put(k, v, lExpireMinutes);
}

Result HDSMClient::erase(const string &k)
{
	return ((HDSMClientImpl *)Impl)->erase(k);
}

Result HDSMClient::exists(const string &k)
{
	return ((HDSMClientImpl *)Impl)->exists(k);
}

Result HDSMClient::length()
{
	return ((HDSMClientImpl *)Impl)->length();
}

HUINT32 HDSMClient::get_sock()
{
	return ((HDSMClientImpl *)Impl)->get_sock();
}

HBOOL HDSMClient::is_valid()
{
	return ((HDSMClientImpl *)Impl)->is_valid();
}

Result HDSMClient::shutdown(const string &password)
{
	return ((HDSMClientImpl *)Impl)->shutdown(password);
}

Result HDSMClient::echo()
{
	return ((HDSMClientImpl *)Impl)->echo();
}

Result HDSMClient::keys(HUINT32 ulLimit /* = 100 */)
{
	return ((HDSMClientImpl *)Impl)->keys(ulLimit);
}