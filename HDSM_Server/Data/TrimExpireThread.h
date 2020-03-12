#pragma once
#include "BaseThread.h"
#include "../Data/IHDSMEventHandler.h"
#include "TypeDefine.h"

class TrimExpireThread
	: public BaseThread
{
public:
	TrimExpireThread(IHDSMEventHandler *pHandler);
	~TrimExpireThread(void);
private:
	virtual void run();
private:
	IHDSMEventHandler	*m_pHandler;
};

