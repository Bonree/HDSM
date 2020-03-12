#pragma once
#include "HDSMCommand.h"
#include "HDSMClient.h"
#include "TestThread.h"
#include <string>

using namespace std;

class HDSMCommandExecutor
{
public:
	HDSMCommandExecutor(HDSMClient *pClient);
	~HDSMCommandExecutor(void);
public:
	HBOOL exec(const string& strRawCommandLine);
public:
	Result get_cur_result();
	HDSMCommand get_cur_raw_command();
	OPRERATE_TYPE get_cur_command_op();
	string get_cur_format_result_info();
private:
	HDSMCommand *m_pCommand;
	HDSMClient	*m_pClient;
	Result		m_Result;
	TestThread *m_pThd;
};

