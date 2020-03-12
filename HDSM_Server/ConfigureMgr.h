#pragma once
#include <string>
#include "TypeDefine.h"

using namespace std;

class ConfigureMgr
{
public:
	ConfigureMgr(){};
	~ConfigureMgr(void){};
public:
	static string get_local_configure_file();
public:
	static string get_data_path();
	static HUINT32 get_max_invalid_keys();
	static HUINT32 get_max_key_length();
	static HUINT32 get_max_value_length();
	static HUINT16 get_listenning_port();
	static HUINT32 get_work_threads_count();
	static HBOOL get_enable_high_level_index();
	static HUINT32 get_syslog_level();
	static string get_syslog_path();
	static HUINT32 get_syslog_max_size();
	static HUINT32 get_trim_expired_keys_period();
	static string get_oplog_path();
	static HUINT32 get_oplog_min_split_period();
	static HUINT32 get_oplog_max_spilt_count();
	static HUINT32 get_oplog_sync_period();
	static HUINT32 get_server_mode();
	static string get_mirror_peer_addr();
	static HUINT32 get_shard_count();
	static HUINT32 get_print_status_info_period();
	static HUINT32 get_init_threads_count();
	static string get_auth_password();
	static HUINT32 get_hour_for_load_expired_keys_cache();
};

