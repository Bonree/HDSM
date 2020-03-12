#ifdef WIN32
#include <windows.h>
#else
#define MAX_PATH 260
#endif

#include "ConfigureMgr.h"
#include "Utils.h"
#include <assert.h>
#include <string.h>
#include "Global.h"

#define KEYVALLEN 256

HCHAR *left_trim(HCHAR *szOutput, const HCHAR *szInput)
{
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	for (; *szInput != '\0' && isspace(*szInput); ++szInput);
	return strcpy(szOutput, szInput);
}

HCHAR *right_trim(HCHAR *szOutput, const HCHAR *szInput)
{
	HCHAR *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	strcpy(szOutput, szInput);
	for (p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p);
	*(++p) = '\0';
	return szOutput;
}

HCHAR *both_trim(HCHAR * szOutput, const HCHAR * szInput)
{
	HCHAR *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	left_trim(szOutput, szInput);
	for (p = szOutput + strlen(szOutput) - 1;p >= szOutput && isspace(*p); --p);
	*(++p) = '\0';
	return szOutput;
}

HINT32 GetProfileStr(const HCHAR *profile, const HCHAR *AppName, const HCHAR *KeyName, HCHAR *KeyVal)
{
	HCHAR appname[32] = {0};
	sprintf(appname, "[%s]", AppName);

	FILE *fp = NULL;
	if ((fp = fopen(profile, "r")) == NULL)
		return -1;

	HCHAR keyname[32] = {0};
	HINT32 found = 0;
	HCHAR buf_i[KEYVALLEN] = {0}, buf_o[KEYVALLEN] = {0};
	while (!feof(fp) && fgets(buf_i, KEYVALLEN, fp)!=NULL)
	{
		left_trim(buf_o, buf_i);
		if (strlen(buf_o) <= 0)
			continue;

		HCHAR *buf = buf_o;
		if (found == 0)
		{
			if (buf[0] != '[')
			{
				continue;
			}
			else if (strncmp(buf, appname, strlen(appname)) == 0)
			{
				found = 1;
				continue;
			}
		} 
		else if (found == 1)
		{
			if (buf[0] == '#' || buf[0] == ';')
			{
				continue;
			}
			else if (buf[0] == '[') 
			{
				break;
			}
			else 
			{
				HCHAR *ch = NULL;
				if ((ch = (HCHAR*)strchr(buf, '=')) == NULL)
					continue;
				memset(keyname, 0, sizeof(keyname));
				sscanf(buf, "%[^=|^ |^\t]", keyname);

				if (strcmp(keyname, KeyName) == 0)
				{
					sscanf(++ch, "%[^\n]", KeyVal);
					HCHAR *KeyVal_o = (HCHAR *)malloc(strlen(KeyVal) + 1);
					if (KeyVal_o != NULL)
					{
						memset(KeyVal_o, 0, strlen(KeyVal) + 1);
						both_trim(KeyVal_o, KeyVal);
						if (KeyVal_o && strlen(KeyVal_o) > 0)
							strcpy(KeyVal, KeyVal_o);
						free(KeyVal_o);
						KeyVal_o = NULL;
					}
					found = 2;
					break;
				} 
				else 
				{
					continue;
				}
			}
		}
	}

	fclose(fp);

	return (found == 2) ? 0 : -1;
}

string GetKeyString(const string &profile, const string &appname, const string &keyname, const string &defaultval)
{
	HCHAR szPath[MAX_PATH] = {0};
#ifdef WIN32
	GetPrivateProfileString(appname.c_str(), keyname.c_str(), defaultval.c_str(), szPath, MAX_PATH, profile.c_str());
	return string(szPath);
#else
	HINT32 ret = GetProfileStr(profile.c_str(), appname.c_str(), keyname.c_str(), szPath);
	return (ret != 0) ? defaultval : string(szPath);
#endif
}

HINT32 GetKeyInt(const string &profile, const string &appname, const string &keyname, HINT32 defaultval)
{
	HCHAR szPath[MAX_PATH] = {0};
#ifdef WIN32
	return GetPrivateProfileInt(appname.c_str(), keyname.c_str(), defaultval, profile.c_str());
#else
	HINT32 ret = GetProfileStr(profile.c_str(), appname.c_str(), keyname.c_str(), szPath);
	return (ret != 0) ? defaultval : atoi(szPath);
#endif
}

string ConfigureMgr::get_local_configure_file()
{
	string cfg;
	Utils::get_current_path(cfg);
	cfg += "/HDSM.ini";
	return cfg;
}

string ConfigureMgr::get_data_path()
{
#ifdef WIN32
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "data_path", "D:/HDSM/data");
#else
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "data_path", "/root/HDSM/data");
#endif
}

HUINT32 ConfigureMgr::get_max_invalid_keys()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_max_invalid_keys", 100000);
}

HUINT32 ConfigureMgr::get_max_key_length()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_max_key_length", 128);
}

HUINT32 ConfigureMgr::get_max_value_length()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_max_value_length", 128);
}

HUINT32 ConfigureMgr::get_shard_count()
{
	HUINT32 cnt = GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_shard_count", 64);
	if (cnt > 128) cnt = 128;
	if (cnt < 16) cnt = 16;
	cnt = cnt / 16 * 16;
	return cnt;
}

HUINT32 ConfigureMgr::get_print_status_info_period()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "srv_print_status_info_period", 60);
}

HUINT16 ConfigureMgr::get_listenning_port()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "srv_listen_port", 9988);
}

HUINT32 ConfigureMgr::get_work_threads_count()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "srv_work_threads_count", Utils::get_cpu_count()*2);
}

HUINT32 ConfigureMgr::get_init_threads_count()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_init_threads_count", Utils::get_cpu_count());
}

HUINT32 ConfigureMgr::get_syslog_level()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "syslog_level", 1);
}

string ConfigureMgr::get_oplog_path()
{
#ifdef WIN32
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "oplog_path", "D:/HDSM/oplog");
#else
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "oplog_path", "/root/HDSM/oplog");
#endif
}

string ConfigureMgr::get_auth_password()
{
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "srv_auth_password", "");
}

string ConfigureMgr::get_syslog_path()
{
#ifdef WIN32
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "syslog_path", "D:/HDSM/syslog");
#else
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "syslog_path", "/root/HDSM/syslog");
#endif
}

HUINT32 ConfigureMgr::get_syslog_max_size()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "syslog_max_size", 1000);
}

HUINT32 ConfigureMgr::get_trim_expired_keys_period()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_trim_expired_keys_period", 60);
}

HUINT32 ConfigureMgr::get_oplog_min_split_period()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "oplog_min_split_period", 5);
}

HUINT32 ConfigureMgr::get_oplog_max_spilt_count()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "oplog_max_split_count", 1000);
}

HUINT32 ConfigureMgr::get_hour_for_load_expired_keys_cache()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "data_hour_for_load_expired_keys_cache", 3);
}

HUINT32 ConfigureMgr::get_oplog_sync_period()
{
	return GetKeyInt(Global::CONFIGURE_FILE_PATH, "configure", "oplog_sync_period", 3);
}

HUINT32 ConfigureMgr::get_server_mode()
{
	string mode = GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "srv_server_mode", "alone");
	return (mode.compare("mirror") == 0 ? 1 : 0);
}

string ConfigureMgr::get_mirror_peer_addr()
{
	return GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "srv_mirror_peer_addr", "");
}

HBOOL ConfigureMgr::get_enable_high_level_index()
{
	string value = GetKeyString(Global::CONFIGURE_FILE_PATH, "configure", "data_enable_high_level_index", "false");
	return (value.compare("true") == 0);
}