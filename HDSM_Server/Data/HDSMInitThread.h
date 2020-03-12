#pragma once
#include "BaseThread.h"
#include "IHDSMEventHandler.h"
#include "TypeDefine.h"

class HDSMInitThread 
	: public BaseThread
{
public:
	HDSMInitThread(IHDSMEventHandler *pHandler, HUINT32 nStartIndex, HUINT32 nEndIndex);
	~HDSMInitThread(void);
private:
	virtual void run();
private:
	IHDSMEventHandler *m_pHandler;
	HUINT32 m_nStartIndex;
	HUINT32 m_nEndIndex;
};

