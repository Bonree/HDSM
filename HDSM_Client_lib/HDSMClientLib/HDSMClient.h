#pragma once
#include <string>
#include "Result.h"
#include "TypeDefine.h"

using namespace std;

class HDSMClient
{
public:
	HDSMClient(const string &strSrvIP, HUINT16 usSrvPort, HINT32 timeout = 5000);
	~HDSMClient(void);
public:
	Result get(const string &k);
	Result put(const string &k, const string &v, HINT32 lExpireMinutes = -1);
	Result update(const string &k, const string &v, HINT32 lExpireMinutes = 0);
	Result erase(const string &k);
	Result exists(const string &k);
	Result length();
	Result shutdown(const string &password);
	Result quit();
	Result echo();
	Result keys(HUINT32 ulLimit = 100);
public:
	HBOOL is_valid();
public:
	HUINT32 get_max_key_len();
	HUINT32 get_max_value_len();
public:
	HUINT32 get_sock();
private:
	void *Impl;
};

