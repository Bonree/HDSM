#pragma once
#include <string>
#include "TypeDefine.h"
using namespace std;

typedef struct tagKeyValueInfo
{
	string key;
	string value;
	HINT32 expire_minutes;
	HUINT64 created_time;

	tagKeyValueInfo(){}

	tagKeyValueInfo(const string &k, const string &v, HINT32 e, HUINT64 t)
	{
		key = k;
		value = v;
		expire_minutes = e;
		created_time = t;
	}
}KeyValueInfo;

enum OPRERATE_TYPE
{
	OP_BEGIN,
	OP_ECHO,
	OP_QUIT,
	OP_GET,
	OP_PUT,
	OP_UPDATE,
	OP_ERASE,
	OP_EXISTS,
	OP_LENGTH,
	OP_SHUTDOWN,
	OP_KEYS,
	OP_ACK,
	OP_SYNC,
	OP_END,
};