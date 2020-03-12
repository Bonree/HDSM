#pragma once
#include "BaseThread.h"
#include "IOplogManager.h"
#include "TypeDefine.h"


class OplogWriteThread 
	: public BaseThread
{
public:
	OplogWriteThread(IOplogManager *pManager);
	virtual ~OplogWriteThread(void);
private:
	virtual void run();
private:
	IOplogManager *m_pManager;
};

