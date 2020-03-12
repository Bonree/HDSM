#include "Logger.h"
#include "SimpleLogger.h"
#include "./ConfigureMgr.h"
#include "./Global.h"
#include <stdarg.h>

HUINT32 SYS_LOG_LEVEL = ConfigureMgr::get_syslog_level();

static SimpleLogger g_SimpleLoggerInst(ConfigureMgr::get_syslog_path().c_str(), SYS_LOG_LEVEL, ConfigureMgr::get_syslog_max_size() * 1024 * 1024);

void Logger::log(HUINT32 level, const HCHAR *level_str, const HCHAR *buffer)
{
	if (level >= SYS_LOG_LEVEL)
		g_SimpleLoggerInst.log(level, level_str, buffer);
	return;
}

void Logger::log_d(const HCHAR *fmt, ...)
{
	if (SimpleLogger::LEVEL_DEBUG < SYS_LOG_LEVEL)
		return;

	HCHAR buffer[Global::DEFAULT_TEMP_BUFFER_SIZE] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	log(SimpleLogger::LEVEL_DEBUG, "DEBUG", buffer);
	return;
}

void Logger::log_e(const HCHAR *fmt, ...)
{
	if (SimpleLogger::LEVEL_ERROR < SYS_LOG_LEVEL)
		return;

	HCHAR buffer[Global::DEFAULT_TEMP_BUFFER_SIZE] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	log(SimpleLogger::LEVEL_ERROR, "ERROR", buffer);
	return;
}

void Logger::log_i(const HCHAR *fmt, ...)
{
	if (SimpleLogger::LEVEL_INFO < SYS_LOG_LEVEL)
		return;

	HCHAR buffer[Global::DEFAULT_TEMP_BUFFER_SIZE] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	log(SimpleLogger::LEVEL_INFO, "INFO", buffer);
	return;
}

void Logger::log_w(const HCHAR *fmt, ...)
{
	if (SimpleLogger::LEVEL_WARNNING < SYS_LOG_LEVEL)
		return;

	HCHAR buffer[Global::DEFAULT_TEMP_BUFFER_SIZE] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	log(SimpleLogger::LEVEL_WARNNING, "WARN", buffer);
	return;
}

void Logger::reload_syslog_level()
{
	SYS_LOG_LEVEL = ConfigureMgr::get_syslog_level();
	return;
}