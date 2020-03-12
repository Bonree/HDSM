#pragma once
#include <string>
#include <vector>
#include "HDSMOperateType_c.h"
#include "TypeDefine.h"
using namespace std;

class Result
{
public:
	Result();
	Result(const string &k);
	Result(const string &k, const string &v);
	~Result(void);
public:
	const string &get_key();
	const string &get_value();
	HUINT32 get_length();
	OPRERATE_TYPE get_operate();
	HUINT32 get_total_time();
	HUINT32 get_process_time();
	HUINT32 get_max_key_length();
	HUINT32 get_max_value_length();
	HUINT32 get_SN();
	HBOOL get_ret();
	HINT32 get_expire_minutes();
	const string &get_err_info();
	vector<KeyValueInfo> get_keys_info();
public:
	HBOOL is_valid();
	HBOOL load(const HCHAR *pBuf, HUINT32 nBufLen);
	void reset();
private:
	void *Impl;
};

