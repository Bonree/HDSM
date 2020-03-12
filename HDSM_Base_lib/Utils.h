#pragma once
#include <string>
#include <fstream>
#include "TypeDefine.h"
using namespace std;

class Utils
{
public:
	Utils(void){};
	~Utils(void){};
public:
	static HBOOL open_file(fstream &fs, const string &filename);
	static HBOOL delete_file(const string &filename);
	static HBOOL rename_file(const string &oldf, const string &newf);
	static HBOOL file_exist(const string &filename);
	static HINT64 get_file_size(const string &filename);
	static HBOOL get_current_path(string &path);
	static HBOOL get_current_path(HCHAR *path, HUINT32 len);
	static HUINT32 get_cpu_count();
	static HUINT8 get_check_sum(const HCHAR *buf, HUINT32 bufLen);
	static HULONG get_tick_count();
	static string get_fmt_local_time();
	static string get_fmt_time(HUINT64 ullTimestamp);
	static HINT32 get_current_hour();
	static HUINT64 get_current_time_stamp_align_minute();
	static HUINT64 get_time_stamp_align_up_minute(HUINT64 ullRawTS);
	static HUINT64 get_current_time_stamp();
	static HBOOL create_dirs(const string &path);
	static void sleep_for_seconds(HUINT32 seconds);
	static void sleep_for_milliseconds(HUINT32 ms);
	static HUINT32 hash(const string &k, HUINT32 len);
	static string trim_cmd_string(const string& str);
};

