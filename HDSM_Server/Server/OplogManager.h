#pragma once
#include "Task.h"
#include "SimpleLock.h"
#include "SimpleNotifier.h"
#include "OplogWriteThread.h"
#include "IOplogManager.h"
#include <string>
#include <queue>
#include <fstream>
#include "TypeDefine.h"

using namespace std;

class OplogManager 
	: public IOplogManager
{
public:
	OplogManager(void);
	virtual ~OplogManager(void);
public:
	HBOOL add_log(Task *pTask);
	HUINT64 sync(string &str);
	HBOOL ack(HUINT64 ack_index);
	HUINT32 get_total_ops();
public:
	virtual HBOOL enter_write_loop();
private:
	HBOOL remove_file(HUINT64 index);
	HBOOL load_write_index();
	HBOOL load_read_index();
	HBOOL update_write_index();
	HBOOL update_read_index(HUINT64 index);
	HBOOL open_new_file();
	string load_file(HUINT64 index);
	void get_log_buffer(Task *pTask, string &strBuffer);
private:
	fstream m_fsWriteIndexFile;
	fstream m_fsReadIndexFile;
	fstream m_fsCurLogFile;
	string m_strOPLogPath;
	string m_strCurOPLogFile;
private:
	HUINT64 m_ullCurWriteIndex;
	HUINT64 m_ullCurReadIndex;
	HUINT32 m_ulLastOpenFileMoment;
	HUINT32 m_uiMinSyncPeriod;
	HUINT32 m_uiMaxSyncOps;
	HUINT32 m_ulWrittenOps;
	HUINT32 m_ulTotalWrittenOps;
private:
	SimpleLock m_lock_for_log_file;
private:
	SimpleLock m_lock_for_write_buffer;
	queue<string> m_WriteBufferQueue;
	SimpleNotifier m_Notifier;
	OplogWriteThread *m_pWriteThread;
};

