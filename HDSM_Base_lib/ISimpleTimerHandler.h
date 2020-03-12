#pragma once
#include "TypeDefine.h"

struct ISimpleTimerHandler 
{
	virtual void on_timer(HUINT32 ulCurTicks) = 0;
};