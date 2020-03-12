#pragma once
#include "TypeDefine.h"

struct IOplogManager
{
	virtual HBOOL enter_write_loop() = 0;
};