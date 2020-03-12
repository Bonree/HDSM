#pragma once
#include "TypeDefine.h"

class Logger
{
public:
	Logger() {};
	virtual ~Logger() {};
public:
	static void reload_syslog_level();
	static void log_d(const HCHAR *fmt, ...);
	static void log_i(const HCHAR *fmt, ...);
	static void log_w(const HCHAR *fmt, ...);
	static void log_e(const HCHAR *fmt, ...);
	static void log(HUINT32 level, const HCHAR *level_str, const HCHAR *buffer);
};

