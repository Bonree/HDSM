#ifdef WIN32
#include <WinSock2.h>  
#endif
#include "TaskCenter.h"
#include "Utils.h"
#include "../ConfigureMgr.h"
#include "../Logger.h"
#include "Task.h"

TaskCenter::TaskCenter(ITaskNotify *pNotify, HUINT32 nWorkThreadCount)
{
	m_pTaskNotify = pNotify;

	string data_path = ConfigureMgr::get_data_path();
	if (!Utils::create_dirs(data_path))
	{
		Logger::log_e("Created Data dir Failed: %s!", data_path.c_str());
		exit(0);
	}

	m_pHDSM = new HDSimpleMap(ConfigureMgr::get_data_path());

	if (Global::SERVER_MODE == Global::SERVER_MODE_MIRROR)
	{
		m_pOPLogger = new OplogManager();
		m_pOplogSyncThd = new OplogSyncThread(this);
	}
	else
	{
		m_pOPLogger = NULL;
		m_pOplogSyncThd = NULL;
	}

	if (nWorkThreadCount == 0)
		m_nThreadCount = Utils::get_cpu_count()*2;
	else
		m_nThreadCount = nWorkThreadCount;
	for (HUINT32 i=0; i<m_nThreadCount; i++)
	{
		WorkThread *pthd = new WorkThread(this);
		if (pthd != NULL)
			m_vecThreads.push_back(pthd);
	}

	Logger::log_i("Created %d work threads!", m_vecThreads.size());
}

TaskCenter::~TaskCenter(void)
{
	for (HUINT32 i=0; i<m_vecThreads.size(); i++)
	{
		WorkThread *pThd = m_vecThreads[i];
		if (pThd != NULL)
		{
			pThd->stop();
			delete pThd;
		}
	}
	m_vecThreads.clear();

	if (m_pOPLogger != NULL)
	{
		delete m_pOPLogger;
		m_pOPLogger = NULL;
	}

	if (m_pOplogSyncThd != NULL)
	{
		m_pOplogSyncThd->stop();
		delete m_pOplogSyncThd;
		m_pOplogSyncThd = NULL;
	}

	if (m_pHDSM != NULL)
		delete m_pHDSM;
}

HBOOL TaskCenter::start()
{
	for (HUINT32 i=0; i<m_vecThreads.size(); i++)
	{
		WorkThread *pThd = m_vecThreads[i];
		if (pThd != NULL)
			pThd->start();
	}

	if (m_pOplogSyncThd != NULL)
		m_pOplogSyncThd->start();
	return true;
}

HBOOL TaskCenter::stop()
{
	for (HUINT32 i=0; i<m_vecThreads.size(); i++)
	{
		WorkThread *pThd = m_vecThreads[i];
		if (pThd != NULL)
			pThd->stop();
	}
	return true;
}

HBOOL TaskCenter::wait_notify()
{
	m_Notifier.wait();
	return true;
}

HBOOL TaskCenter::push_task(const Task &task)
{
	if (task.is_valid())
	{
		m_lock.lock();
		Logger::log_d("Recv New Task[%s], %d Tasks In the Center", task.get_operate_str().c_str(), m_TaskQueue.size());
		m_TaskQueue.push(task);
		m_Notifier.awake();
		m_lock.unlock();	
	}

	return true;
}

Task TaskCenter::pop_task()
{
	Task task; 
	m_lock.lock();
	if (m_TaskQueue.size() > 0)
	{
		task = m_TaskQueue.front();
		m_TaskQueue.pop();
		if (m_TaskQueue.size() == 0)
			m_Notifier.rest();
	}
	m_lock.unlock();
	return task;
}

HBOOL TaskCenter::exec_task(Task &task)
{
	Task *pTask = &task;
	if (pTask != NULL && m_pHDSM != NULL)
	{
		HBOOL bRet = false;
		string v;
		string data;
		HINT32 lExpireMinutes = -1;
		HUINT64 ullAckIndex = 0;
		map<string, KeyValueInfo> KeysInfo;
		pTask->record_start_moment();
		switch (pTask->get_operate())
		{
		case OP_ECHO:
			bRet = true;
			pTask->set_ret(bRet);
			Logger::log_d("EXEC TASK [ECHO] [T], processtime= %dms, totaltime= %dms", pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_SHUTDOWN:
			bRet = (pTask->get_password().compare(ConfigureMgr::get_auth_password()) == 0);
			pTask->set_ret(bRet);
			if (!bRet)
				pTask->set_err_info("Password is uncorrect!");
			Logger::log_d("EXEC TASK [SHUTDOWN] [T], processtime= %dms, totaltime= %dms", pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_ACK:
			if (m_pOPLogger != NULL)
				m_pOPLogger->ack(pTask->get_ack_index());
			bRet = true;
			pTask->set_ret(bRet);
			Logger::log_d("EXEC TASK [ACK] [T], processtime= %dms, totaltime= %dms", pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_SYNC:
			if (m_pOPLogger != NULL)
			{
				ullAckIndex = m_pOPLogger->sync(data);
				if (ullAckIndex > 0)
				{
					bRet = true;
					pTask->set_sync_data(data);
					pTask->set_ack_index(ullAckIndex);
				}
				else
				{
					bRet = (data.compare("NODATA") == 0);
					pTask->set_sync_data("");
					pTask->set_ack_index(0);
				}
			}
			else
			{
				Logger::log_w("some Mirror peer is connecting!But the Server is running in ALONE mode!");
				pTask->set_sync_data("");
				pTask->set_ack_index(0);
			}
			pTask->set_ret(bRet);
			Logger::log_d("EXEC TASK [SYNC] [%s], processtime= %dms, totaltime= %dms", bRet ? "T":"F", pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_QUIT:
			bRet = true;
			pTask->set_ret(bRet);
			Logger::log_d("EXEC TASK [QUIT] [T], processtime= %dms, totaltime= %dms", pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_GET:		
			bRet = m_pHDSM->get(pTask->get_key(), v, lExpireMinutes);
			if (!bRet)
			{
				Logger::log_d("Get Key[%s] failed!", pTask->get_key().c_str());
				pTask->set_err_info("The key is not exist!");
			}
			pTask->set_ret(bRet);
			if (bRet)
			{
				pTask->set_value(v);
				pTask->set_expire_minutes(lExpireMinutes);
			}
			Logger::log_d("EXEC TASK [GET] [%s],k=%s, v=%s, processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_key().c_str(), v.c_str(), pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_UPDATE:
			bRet = m_pHDSM->update(pTask->get_key(), pTask->get_value(), pTask->get_time_stamp(), pTask->get_expire_minutes());
			if (bRet && m_pOPLogger!=NULL && pTask->get_conn() != NULL)//没有连接信息的是同步过来的数据
				m_pOPLogger->add_log(pTask);
			pTask->set_ret(bRet);
			if (!bRet)
				pTask->set_err_info("The key is not exist!");
			Logger::log_d("EXEC TASK [UPDATE] [%s],k=%s, v=%s, processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_key().c_str(), pTask->get_value().c_str(), pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_PUT:
			if (pTask->get_expire_minutes() > 0 && Utils::get_current_time_stamp() > pTask->get_time_stamp())
			{
				//已经超时的，就不要再放了
				if (Utils::get_current_time_stamp() - pTask->get_time_stamp() >= pTask->get_expire_minutes() * 60)
				{
					pTask->set_ret(true);
					Logger::log_d("TASK [PUT] k=%s, v=%s Canceled!", pTask->get_key().c_str(), pTask->get_value().c_str());
					break;
				}
			}

			bRet = m_pHDSM->put(pTask->get_key(), pTask->get_value(), pTask->get_time_stamp(), pTask->get_expire_minutes());
			if (bRet && m_pOPLogger!=NULL && pTask->get_conn() != NULL)//没有连接信息的是同步过来的数据
				m_pOPLogger->add_log(pTask);
			pTask->set_ret(bRet);
			if (!bRet)
				pTask->set_err_info("The key is already exist!");
			Logger::log_d("EXEC TASK [PUT] [%s],k=%s, v=%s, processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_key().c_str(), pTask->get_value().c_str(), pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_ERASE:
			bRet = m_pHDSM->erase(pTask->get_key());
			if (bRet && m_pOPLogger!=NULL && pTask->get_conn() != NULL)//没有连接信息的是同步过来的数据
				m_pOPLogger->add_log(pTask);
			pTask->set_ret(bRet);
			if (!bRet)
				pTask->set_err_info("The key is not exist!");
			Logger::log_d("EXEC TASK [ERASE] [%s],k=%s, processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_key().c_str(), pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_EXISTS:
			bRet = m_pHDSM->exists(pTask->get_key());
			pTask->set_ret(bRet);
			if (!bRet)
				pTask->set_err_info("The key is not exist!");
			Logger::log_d("EXEC TASK [EXISTS] [%s],k=%s, processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_key().c_str(), pTask->get_process_time(), pTask->get_total_time());
			break;
		case OP_LENGTH:
			bRet = true;
			pTask->set_ret(bRet);
			pTask->set_length(m_pHDSM->length());
			Logger::log_d("EXEC TASK [LENGTH] [%s],count=%d, processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_length(), pTask->get_process_time(), pTask->get_total_time());		
			break;
		case OP_KEYS:
			bRet = true;
			pTask->set_ret(bRet);
			map<string, KeyValueInfo> m = m_pHDSM->keys(pTask->get_show_keys_limit());
			pTask->set_keys_info(m);
			//pTask->set_keys_info(m_pHDSM->keys(pTask->get_show_keys_limit()));
			Logger::log_d("EXEC TASK [KEYS] [%s],processtime= %dms, totaltime= %dms",
				bRet ? "T":"F", pTask->get_process_time(), pTask->get_total_time());	
			break;
		}

		//如果连接是空的，说明这是个oplog同步过来的任务，不需要给客户端响应
		if (pTask->get_conn() != NULL)
		{
			Connection::Response resp(pTask->buffer(), pTask->get_operate());
			pTask->get_conn()->push_response(resp);
			if (m_pTaskNotify != NULL)
			{
				m_pTaskNotify->on_task_completed(pTask->get_conn());
				if (bRet && pTask->get_operate() == OP_SHUTDOWN)
				{
					m_pTaskNotify->on_command(pTask->get_operate());
				}
			}
		}
	}

	return true;
}

HUINT32 TaskCenter::get_task_count()
{
	return m_TaskQueue.size();
}

void TaskCenter::renotify()
{
	m_Notifier.awake();
}

string TaskCenter::get_indexs_size()
{
	HCHAR szInfo[64] = {0};
	vector<HUINT32> vec = m_pHDSM->get_indexs_size();
	sprintf(szInfo, "%d | %d | %d | %d | %d", vec[0], vec[1], vec[2], vec[3], vec[4]);
	return szInfo;
}

HUINT32 TaskCenter::get_expire_cache_size()
{
	return m_pHDSM->get_expire_cache_size();
}

HUINT32 TaskCenter::get_total_keys()
{
	return m_pHDSM->length();
}

HUINT32 TaskCenter::get_total_oplogs()
{
	HUINT32 ops = 0;
	if (m_pOPLogger != NULL)
		ops = m_pOPLogger->get_total_ops();
	return ops;
}