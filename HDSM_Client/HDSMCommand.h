#pragma once
#include <string>
#include "HDSMOperateType_c.h"
using namespace std;

class HDSMCommand
{
public:
	HDSMCommand();
	HDSMCommand(const string &sRawCommand);
	virtual ~HDSMCommand(void);
public:
	OPRERATE_TYPE get_test_op_type() const;
	HUINT32 get_test_start_offset() const;
	HUINT32 get_test_end_offset() const;
	OPRERATE_TYPE get_op_type() const;
	const string &get_key();
	const string &get_value();
	const string &get_password();
	HINT32 get_expire_minutes();
	HBOOL is_valid() const;
	HUINT32 get_show_keys_limit();
private:
	HBOOL parse_put_or_update_command(const string &cmd);
	HBOOL parse_get_or_exists_or_erase_command(const string &cmd);
	HBOOL parse_shutdown_command(const string &cmd);
	HBOOL parse_showkeys_command(const string &cmd);
	HBOOL parse_test_command(const string &cmd);
private:
	OPRERATE_TYPE m_ulOperate;
	string		m_strKey;
	string		m_strValue;
	HINT32		m_lExpireMinutes;
	string		m_strPassword;
	HBOOL		m_bValid;
	HUINT32		m_ulShowKeysLimit;
	HUINT32		m_ulStartOffset;
	HUINT32		m_ulEndOffset;
	OPRERATE_TYPE m_ulTestOperate;
};

