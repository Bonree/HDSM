#pragma once
#include <string>
#include <map>
#include "Connection.h"
#include "../Data/HDSMOperateType.h"
#include "../Data/IHDLinkedList.h"
#include "TypeDefine.h"

using namespace std;

class Task
{
public:
	//Task(Connection *pConn, OPRERATE_TYPE op, string k/* = ""*/, string v/* = ""*/, HINT32 lExpireMinutes);
	Task();
	Task(const Task &t);
	Task(Connection *pConn, const HCHAR *pszBuf, HUINT32 iBufLen);
	~Task(void);
public:
	const string &get_key();
	const string &get_value();
	HUINT32 get_length();
	OPRERATE_TYPE get_operate();
	string get_operate_str() const;
	HUINT32 get_sn();
	HINT32 get_expire_minutes();
	HUINT32 get_show_keys_limit();
	HUINT64 get_time_stamp();
	HUINT64 get_ack_index();
	const string &get_password();
public:
	void set_ret(HBOOL r);
	void set_value(const string &v);
	void set_length(HUINT32 len);
	void set_expire_minutes(HINT32 lExpireMinutes);
	void set_sync_data(const string &data);
	void set_ack_index(HUINT64 ullAckIndex);
	void set_err_info(const string &info);
	void set_keys_info(map<string, KeyValueInfo> &keys);
public:
	Connection *get_conn();
	HBOOL is_valid() const;
public:
	HUINT32 get_total_time();
	HUINT32 get_process_time();
	void record_start_moment();
public:
	string buffer();
private:
	HBOOL	m_bValid;
private:
	HUINT32	m_bRet;
	string	m_strKey;
	string	m_strValue;
	HUINT32	m_ulLength;
	HUINT32	m_ulOperate;
	HUINT32	m_ulSN;
	HINT32	m_lExpireMinutes;
	string	m_strPassword;
private:
	Connection		*m_pConn;
private:
	HUINT32	m_ulRecvMoment;
	HUINT32	m_ulStartMoment;
	HUINT32	m_ulFinishMoment;
private:
	HUINT64 m_ullTimeStamp;
private:
	HUINT64 m_ullAckIndex;
	string	m_strSyncData;
private:
	string	m_strErrInfo;
	map<string, KeyValueInfo> m_KeysInfo;
	HUINT32	m_ulShowKeysLimit;
};

