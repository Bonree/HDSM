#include "HDSMCommandExecutor.h"
#include "Utils.h"
#include <string.h>

HDSMCommandExecutor::HDSMCommandExecutor(HDSMClient *pClient)
{
	m_pCommand = NULL;
	m_pClient = pClient;
	m_pThd = NULL;
}

HDSMCommandExecutor::~HDSMCommandExecutor(void)
{
	if (m_pCommand != NULL)
	{
		delete m_pCommand;
		m_pCommand = NULL;
	}

	if (m_pThd != NULL)
	{
		m_pThd->stop();
		delete m_pThd;
		m_pThd = NULL;
	}
}

HBOOL HDSMCommandExecutor::exec(const string& strRawCommandLine)
{
	if (m_pClient == NULL || !m_pClient->is_valid())
		return false;

	if (m_pCommand != NULL)
	{
		delete m_pCommand;
		m_pCommand = NULL;
	}

	m_pCommand = new HDSMCommand(strRawCommandLine);

	if (m_pCommand == NULL)
		return false;
	
	if (!m_pCommand->is_valid())
	{
		delete m_pCommand;
		m_pCommand = NULL;
		return false;
	}
	
	switch (m_pCommand->get_op_type())
	{
	case OP_GET:
		m_Result = m_pClient->get(m_pCommand->get_key());
		break;
	case OP_PUT:
		m_Result = m_pClient->put(m_pCommand->get_key(), m_pCommand->get_value(), m_pCommand->get_expire_minutes());
		break;
	case OP_UPDATE:
		m_Result = m_pClient->update(m_pCommand->get_key(), m_pCommand->get_value(), m_pCommand->get_expire_minutes());
		break;
	case OP_LENGTH:
		m_Result = m_pClient->length();
		break;
	case OP_EXISTS:
		m_Result = m_pClient->exists(m_pCommand->get_key());
		break;
	case OP_ERASE:
		m_Result = m_pClient->erase(m_pCommand->get_key());
		break;
	case OP_QUIT:
		m_Result = m_pClient->quit();
		break;
	case OP_SHUTDOWN:
		m_Result = m_pClient->shutdown(m_pCommand->get_password());
		break;
	case OP_ECHO:
		m_Result = m_pClient->echo();
		break;
	case OP_KEYS:
		m_Result = m_pClient->keys(m_pCommand->get_show_keys_limit());
		break;
	case OP_TEST:
		if (m_pThd != NULL)
		{
			m_pThd->stop();
			delete m_pThd;
			m_pThd = NULL;
		}

		m_pThd = new TestThread(m_pClient, m_pCommand->get_test_op_type());
		if (m_pThd != NULL)
		{
			m_pThd->set_key_offset_interval(m_pCommand->get_test_start_offset(), m_pCommand->get_test_end_offset());
			m_pThd->start();
		}
		break;
	default:
		return false;
	}

	return true;
}

Result HDSMCommandExecutor::get_cur_result()
{
	return m_Result;
}

HDSMCommand HDSMCommandExecutor::get_cur_raw_command()
{
	if (m_pCommand != NULL)
		return *m_pCommand;
	else
		return HDSMCommand();
}

OPRERATE_TYPE HDSMCommandExecutor::get_cur_command_op()
{
	return get_cur_raw_command().get_op_type();
}

string HDSMCommandExecutor::get_cur_format_result_info()
{
	string strInfo;
	HCHAR szInfo[4096] = {0};
	if (m_Result.is_valid())
	{
		if (!m_Result.get_ret())
		{
			if (m_pCommand->get_op_type()!= OP_EXISTS)
				sprintf(szInfo, "Result status: Failed, Processed in %d ms and Total time is %d ms.\nerror info: %s\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time(),
					m_Result.get_err_info().c_str());
			else
				sprintf(szInfo, "Result status: Failed, Processed in %d ms and Total time is %d ms.\nExists: False\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time());
			strInfo = szInfo;
		}
		else
		{
			switch (m_pCommand->get_op_type())
			{
			case OP_GET:
				sprintf(szInfo, "Result status: Successed, Processed in %d ms and Total time is %d ms.\nValue: %s\nexpire: %d min(s)\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time(),
					m_Result.get_value().c_str(),
					m_Result.get_expire_minutes());
				strInfo = szInfo;
				break;
			case OP_PUT:
			case OP_ERASE:
			case OP_UPDATE:
			case OP_QUIT:
			case OP_SHUTDOWN:
				sprintf(szInfo, "Result status: Successed, Processed in %d ms and Total time is %d ms.\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time());
				strInfo = szInfo;
				break;
			case OP_LENGTH:
				sprintf(szInfo, "Result status: Successed, Processed in %d ms and Total time is %d ms.\nHDSM Length: %d\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time(),
					m_Result.get_length());
				strInfo = szInfo;
				break;
			case OP_EXISTS:
				sprintf(szInfo, "Result status: Successed, Processed in %d ms and Total time is %d ms.\nExists: True\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time());
				strInfo = szInfo;
				break;
			case OP_ECHO:
				sprintf(szInfo, "Result status: Successed, Processed in %d ms and Total time is %d ms.\nmax_key_len: %d\nmax_value_len: %d\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time(),
					m_Result.get_max_key_length(),
					m_Result.get_max_value_length());
				strInfo = szInfo;
				break;
			case OP_KEYS:
				sprintf(szInfo, "Result status: Successed, Processed in %d ms and Total time is %d ms.\n", 
					m_Result.get_process_time(), 
					m_Result.get_total_time());
				strInfo = szInfo;
				
				memset(szInfo, 0, 4096);
				sprintf(szInfo, "\n     %-20s%-20s%-8s%-12s\n------------------------------------------------------------------------\n",
					"KEY", "VALUE", "EXPIRE", "CREATED");
				strInfo += szInfo;

				vector<KeyValueInfo> vec = m_Result.get_keys_info();
				for (HUINT32 i=0; i<vec.size(); i++)
				{
					memset(szInfo, 0, 4096);
					sprintf(szInfo, "%-5d%-20s%-20s%-8d%-12s\n", i+1,
						vec[i].key.c_str(), vec[i].value.c_str(), vec[i].expire_minutes, Utils::get_fmt_time(vec[i].created_time).c_str());
					strInfo += szInfo;
				}
				strInfo += "------------------------------------------------------------------------\n";
				break;
			}
		}
	}

	return strInfo;
}