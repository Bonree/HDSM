#pragma once
#include "TypeDefine.h"

struct IHDSMEventHandler 
{
	virtual HUINT32 expire() = 0;
	virtual HUINT32 length() = 0;
	virtual HBOOL init_some_hdlists(HUINT32 nStartIndex, HUINT32 nEndIndex) = 0;
	virtual HBOOL load_expire_cache() = 0;
};