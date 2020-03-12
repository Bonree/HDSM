#pragma once
#include "Task.h"
#include "TypeDefine.h"

struct ITaskCenter
{
	virtual HBOOL push_task(const Task &) = 0;
	virtual Task pop_task() = 0;
	virtual HBOOL exec_task(Task &) = 0;
	virtual HBOOL wait_notify() = 0;
};