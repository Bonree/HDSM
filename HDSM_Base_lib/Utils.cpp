#ifdef WIN32
#include <WinSock2.h>  
#include <Windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#define MAX_PATH 260
#endif

#include "Utils.h"
#include <time.h>


HBOOL Utils::open_file(fstream &fs, const string &filename)
{
	fs.open(filename, ios::in|ios::out|ios::binary);
	if (fs.is_open()) return true;
	fs.open(filename, ios::out|ios::binary);
	if (!fs.is_open()) return false;
	fs.close();
	fs.open(filename, ios::in|ios::out|ios::binary);
	return fs.is_open();
}

HBOOL Utils::delete_file(const string &filename)
{
#ifdef WIN32
	return DeleteFile(filename.c_str()) ? true : false;
#else
	return (unlink(filename.c_str()) == 0);
#endif
}

HBOOL Utils::rename_file(const string &oldf, const string &newf)
{
	return (rename(oldf.c_str(), newf.c_str()) == 0);
}

HBOOL Utils::file_exist(const string &filename)
{
	return (access(filename.c_str(), 0) != -1);
}

HBOOL Utils::get_current_path(string &path)
{   
	HCHAR szPath[260] = {0};
#ifdef WIN32
	if (0 == GetModuleFileName(GetModuleHandle(NULL), szPath, 259)) 
		return false;
#else
	if (readlink("/proc/self/exe", szPath, 259) < 0)
		return false;
#endif

	HCHAR* p = szPath;
	while (*p) ++p;
	while('\\' != *p && '/' != *p) --p;
	*p = '\0';

	path = szPath;
	return true;
}

HBOOL Utils::get_current_path(HCHAR *path, HUINT32 len)
{
#ifdef WIN32
	if (0 == GetModuleFileName(GetModuleHandle(NULL), path, len-1)) 
		return false;
#else
	if (readlink("/proc/self/exe", path, len-1) < 0)
		return false;
#endif

	HCHAR* p = path;
	while (*p) ++p;
	while('\\' != *p) --p;
	*p = '\0';

	return true;
}

HUINT32 Utils::get_cpu_count()
{
#ifdef WIN32
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	PGNSI pfnGNSI = (PGNSI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo");
	SYSTEM_INFO si;
	if (pfnGNSI != NULL) 
		pfnGNSI(&si);
	else
		GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
#else
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

HUINT8 Utils::get_check_sum(const HCHAR *buf, HUINT32 bufLen)
{
	HUINT8 Sum = 0;
	for (HUINT32 i=0; i<bufLen; i++)
		Sum += (HUINT8)buf[i];
	Sum = ~Sum + 1;
	return Sum;
}

HULONG Utils::get_tick_count()
{
#ifdef WIN32
	return GetTickCount();
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}

string Utils::get_fmt_time(HUINT64 ullTimestamp)
{
	time_t t = ullTimestamp;
	struct tm *p = localtime(&t);

	HCHAR szTime[64] = {0};
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", 
		p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	return string(szTime);
}

string Utils::get_fmt_local_time()
{
	time_t now;
	time(&now);

	struct tm *tm_now = localtime(&now);
	HCHAR szTime[64] = {0};
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", 
		tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

	return string(szTime);
}

HUINT64 Utils::get_current_time_stamp_align_minute()
{
	time_t now;
	time(&now);

	struct tm *tm_now = localtime(&now);
	now -= tm_now->tm_sec;
	
	return (HUINT64)now;
}

HUINT64 Utils::get_time_stamp_align_up_minute(HUINT64 ullRawTS)
{
	time_t t = ullRawTS;
	struct tm *p = localtime(&t);
	return (HUINT64)(t + 60 - p->tm_sec);
}

HINT32 Utils::get_current_hour()
{
	time_t now;
	time(&now);

	struct tm *tm_now = localtime(&now);
	return tm_now->tm_hour;
}

HUINT64 Utils::get_current_time_stamp()
{
	return (HUINT64)time(NULL);
}

HBOOL Utils::create_dirs(const string &path)
{
	HUINT32 dirPathLen = path.length();
	if (dirPathLen > MAX_PATH)
		return false;
	
	HCHAR tmpDirPath[MAX_PATH] = { 0 };
	for (HUINT32 i = 0; i < dirPathLen; ++i)
	{
		tmpDirPath[i] = path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (access(tmpDirPath, 0) == -1)
			{
#ifdef WIN32
				if (!CreateDirectory(tmpDirPath, NULL))
					return false;
#else
				if (mkdir(tmpDirPath, 0777) != 0)
					return false;
#endif
			}
		}
	}

	if (access(tmpDirPath, 0) == -1)
	{
#ifdef WIN32
		if (!CreateDirectory(tmpDirPath, NULL))
			return false;
#else
		if (mkdir(tmpDirPath, 0777) != 0)
			return false;
#endif
	}

	return true;
}

void Utils::sleep_for_seconds(HUINT32 seconds)
{
#ifdef WIN32
	::Sleep(seconds * 1000);
#else
	sleep(seconds);
#endif
	return;
}

void Utils::sleep_for_milliseconds(HUINT32 ms)
{
#ifdef WIN32
	::Sleep(ms);
#else
	usleep(ms*1000);
#endif
	return;
}

HINT64 Utils::get_file_size(const string &filename)
{
	HINT64 size = -1;
	ifstream ifs(filename);
	if (ifs.is_open())
	{
		ifs.seekg(0, ios::end);
		size = ifs.tellg();
	}
	return size;
}

HUINT32 Utils::hash(const string &k, HUINT32 len)
{
	HUINT32 h = 0;
	for (HUINT32 i=0; i<k.size() && i <len; i++) 
	{ 
		h *= 16777619; 
		h ^= (HUINT32) (k.c_str()[i]); 
	}
	return h; 
}

string Utils::trim_cmd_string(const string& str)
{
	string::size_type pos = str.find_first_not_of(' ');
	string cmd;
	if (pos != string::npos)
		cmd = str.substr(pos, str.find_last_not_of(' ') - pos + 1);
	pos = -1;
	while ((pos = cmd.find(' ', pos + 1)) != string::npos)
	{
		while (cmd.at(pos + 1) == ' ')
			cmd.erase(pos + 1, 1);
	}

	pos = 0;
	while (pos < cmd.length())
	{
		if (cmd.at(pos) == '\r' || cmd.at(pos) == '\n')
			cmd.erase(pos, 1);
		else
			pos++;
	}
	return cmd;
}