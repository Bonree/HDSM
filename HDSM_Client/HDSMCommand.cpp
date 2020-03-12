#include "HDSMCommand.h"
#include "Utils.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

HDSMCommand::HDSMCommand()
{
	m_ulOperate = OP_BEGIN;
	m_bValid = false;
	m_ulEndOffset = m_ulStartOffset = 0;
}

HDSMCommand::HDSMCommand(const string &sRawCommand)
{
	//put key value expire
	//update key value expire
	//get key
	//exists key
	//erase key
	//length
	//shutdown password
	//quit
	//echo
	//showkeys 100
	//test [put/get/erase] startoffset endoffset 
	m_bValid = false;
	string cmd = Utils::trim_cmd_string(sRawCommand);
	if (cmd.length() > 0)
	{
		string::size_type pos1 = cmd.find(' ');
		if (pos1 == string::npos)
		{
			if (cmd.compare("length") == 0)
			{
				m_ulOperate = OP_LENGTH;
				m_bValid = true;
			}
			else if (cmd.compare("quit") == 0)
			{
				m_ulOperate = OP_QUIT;
				m_bValid = true;
			}
			else if (cmd.compare("echo") == 0)
			{
				m_ulOperate = OP_ECHO;
				m_bValid = true;
			}
			else if (cmd.compare("showkeys") == 0)
			{
				m_ulOperate = OP_KEYS;
				m_bValid = true;
				m_ulShowKeysLimit = 100;
			}
			else
			{
				cout << "Invalid Command : " << cmd.c_str() << endl;
			}
		}
		else
		{
			string op = cmd.substr(0, pos1);
			if (op.compare("put") == 0)
			{
				m_ulOperate = OP_PUT;
				parse_put_or_update_command(cmd);
			}
			else if (op.compare("update") == 0)
			{
				m_ulOperate = OP_UPDATE;
				parse_put_or_update_command(cmd);
			}
			else if (op.compare("get") == 0)
			{
				m_ulOperate = OP_GET;
				parse_get_or_exists_or_erase_command(cmd);
			}
			else if (op.compare("exists") == 0)
			{
				m_ulOperate = OP_EXISTS;
				parse_get_or_exists_or_erase_command(cmd);
			}
			else if (op.compare("erase") == 0)
			{
				m_ulOperate = OP_ERASE;
				parse_get_or_exists_or_erase_command(cmd);
			}
			else if (op.compare("shutdown") == 0)
			{
				m_ulOperate = OP_SHUTDOWN;
				parse_shutdown_command(cmd);
			}
			else if (op.compare("showkeys") == 0)
			{
				m_ulOperate = OP_KEYS;
				parse_showkeys_command(cmd);
			}
			else if (op.compare("test") == 0)
			{
				m_ulOperate = OP_TEST;
				parse_test_command(cmd);
			}
			else
			{
				cout << "Invalid Command : " << cmd.c_str() << endl;
			}
		}
	}
}


HDSMCommand::~HDSMCommand(void)
{
}

HBOOL HDSMCommand::parse_showkeys_command(const string &cmd)
{
	string::size_type pos1 = cmd.find(' ');
	if (pos1 != string::npos)
	{
		string slimit = cmd.substr(pos1+1, cmd.length()-pos1-1);
		if (slimit.length() > 0)
		{
			m_ulShowKeysLimit = atol(slimit.c_str());
			if (m_ulShowKeysLimit == 0)
				m_ulShowKeysLimit = 100;
			if (m_ulShowKeysLimit > 10000)
				m_ulShowKeysLimit = 10000;
			m_bValid = true;
		}
	}

	return m_bValid;
}

HBOOL HDSMCommand::parse_put_or_update_command(const string &cmd)
{
	string::size_type pos1 = cmd.find(' ');
	string::size_type pos2 = cmd.find(' ', pos1+1);
	if (pos2 != string::npos)
	{
		string key = cmd.substr(pos1+1, pos2-pos1-1);
		if (key.length() > 0)
		{
			m_strKey = key;
			string::size_type pos3 = cmd.find(' ', pos2+1);
			if (pos3 != string::npos)
			{
				string value = cmd.substr(pos2+1, pos3-pos2-1);
				if (value.length() > 0)
				{
					m_strValue = value;				
					string::size_type pos4 = cmd.find(' ', pos3+1);
					if (pos4 == string::npos)
						m_lExpireMinutes = atol(cmd.substr(pos3+1, cmd.size()-pos3-1).c_str());
					else
						m_lExpireMinutes = atol(cmd.substr(pos3+1, pos4-pos3-1).c_str());
					m_bValid = true;
				}
			}
			else
			{
				m_strValue = cmd.substr(pos2+1, cmd.length()-pos2-1);
				m_lExpireMinutes = -1;
				m_bValid = true;
			}
		}
	}

	return m_bValid;
}

HBOOL HDSMCommand::parse_get_or_exists_or_erase_command(const string &cmd)
{
	string::size_type pos1 = cmd.find(' ');
	if (pos1 != string::npos)
	{
		string key = cmd.substr(pos1+1, cmd.length()-pos1-1);
		if (key.length() > 0)
		{
			m_strKey = key;
			m_bValid = true;
		}
	}

	return m_bValid;
}

HBOOL HDSMCommand::parse_shutdown_command(const string &cmd)
{
	string::size_type pos1 = cmd.find(' ');
	if (pos1 != string::npos)
	{
		string pwd = cmd.substr(pos1+1, cmd.length()-pos1-1);
		if (pwd.length() > 0)
		{
			m_strPassword = pwd;
			m_bValid = true;
		}
	}

	return m_bValid;
}

HBOOL HDSMCommand::parse_test_command(const string &cmd)
{
	string::size_type pos1 = cmd.find(' ');
	string::size_type pos2 = cmd.find(' ', pos1+1);
	if (pos2 != string::npos)
	{
		string op = cmd.substr(pos1+1, pos2-pos1-1);
		if (op.length() > 0)
		{
			if (op.compare("put") == 0)
				m_ulTestOperate = OP_PUT;
			else if (op.compare("get") == 0)
				m_ulTestOperate = OP_TEST;
			else if (op.compare("erase") == 0)
				m_ulTestOperate = OP_ERASE;
			else
				return false;

			string::size_type pos3 = cmd.find(' ', pos2+1);
			if (pos3 != string::npos)
			{
				string strStartOffset = cmd.substr(pos2+1, pos3-pos2-1);
				if (strStartOffset.length() > 0)
				{
					m_ulStartOffset = atol(strStartOffset.c_str());

					string strEndOffset = cmd.substr(pos3+1, cmd.length()-pos3-1).c_str();
					if (strEndOffset.length() > 0)
						m_ulEndOffset = atol(strEndOffset.c_str());

					if (m_ulEndOffset <= 100000000 && m_ulEndOffset > m_ulStartOffset)
						m_bValid = true;
				}
			}
		}
	}

	return m_bValid;
}

OPRERATE_TYPE HDSMCommand::get_test_op_type() const
{
	return m_ulTestOperate;
}

HUINT32 HDSMCommand::get_test_start_offset() const
{
	return m_ulStartOffset;
}

HUINT32 HDSMCommand::get_test_end_offset() const
{
	return m_ulEndOffset;
}

OPRERATE_TYPE HDSMCommand::get_op_type() const
{
	return m_ulOperate;
}

const string &HDSMCommand::get_key()
{
	return m_strKey;
}

const string &HDSMCommand::get_value()
{
	return m_strValue;
}

const string &HDSMCommand::get_password()
{
	return m_strPassword;
}

HINT32 HDSMCommand::get_expire_minutes()
{
	return m_lExpireMinutes;
}

HBOOL HDSMCommand::is_valid() const
{
	return m_bValid;
}

HUINT32 HDSMCommand::get_show_keys_limit()
{
	return m_ulShowKeysLimit;
}