#pragma once
#include "Connection.h"
#include "TypeDefine.h"

struct ITaskNotify
{
	virtual HBOOL on_task_completed(Connection *pConn) = 0;
	virtual HBOOL on_command(HUINT32 cmd) = 0;
};