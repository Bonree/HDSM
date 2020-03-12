#include "Utils.h"
#include "BaseSocket.h"
#include "./Server/TaskServer.h"
#include "TestThread.h"
#include "Global.h"
#include "./Logger.h"
#include "TypeDefine.h"

#define _SWITCH_FOR_TEST_ 0

HINT32 main(HINT32 argc, HCHAR* argv[])
{
#if _SWITCH_FOR_TEST_
	HDSimpleMap *m_pHDSM = NULL;
	string data_path = ConfigureMgr::get_data_path();
	Utils::create_dirs(data_path);

	m_pHDSM = new HDSimpleMap(ConfigureMgr::get_data_path());

	for (HINT32 i=0; i<5; i++)
	{
		TestThread *pthread = new TestThread(m_pHDSM);
		pthread->set_interval(i*100000, (i+1)*100000);
		pthread->start();
		Sleep(1);
	}

	system("pause");
#else
		BaseSocket::init();
		Logger::log_i("Welcome to use HDSM %d.0!!!", Global::VERSION_CODE);
		Logger::log_i("Server Work Mode: %s", Global::SERVER_MODE == 0 ? "ALONE" : "MIRROR");
		Logger::log_i("Location of Data: %s", ConfigureMgr::get_data_path().c_str());
		Logger::log_i("Count of Data Shards: %d", Global::MAX_SHARD_COUNT);
		Logger::log_i("Server is loading Data Index(a few minutes)...");

		TaskServer server(ConfigureMgr::get_listenning_port());
		server.start();
		
		BaseSocket::clean();
#endif
	return 0;
}