// HDSM_Client.cpp : 定义控制台应用程序的入口点。
//

#include <map>
#include <iostream>
#include <string.h>
#include "HDSMClient.h"
#include "HDSMCommandExecutor.h"
#include "Result.h"
#include <string.h>

using namespace std;

const HUINT32 BUFFER_SIZE = 1024;

void print_command_help_info()
{
	cout << "HDSMClient Offers Commands: " <<endl;
	cout << "----------------------------" <<endl;
	cout << "1. put key value [expire(min)]" <<endl;
	cout << "2. update key value [expire(min)]" <<endl;
	cout << "3. get key" <<endl;
	cout << "4. exists key" <<endl;
	cout << "5. erase key" <<endl;
	cout << "6. length" <<endl;
	cout << "7. shutdown password" <<endl;
	cout << "8. quit" <<endl;
	cout << "9. echo" <<endl;
	cout << "10. showkeys [limit]" <<endl;
	cout << "11. test op[put/get/erase] startoffset endoffset" <<endl;
	cout << "----------------------------" <<endl << endl;
	return;
}

HINT32 main(HINT32 argc, HCHAR* argv[])
{
	if (argc < 3)
	{
		cout << "Invalid Command Line!" << endl;
		return -1;
	}

	HCHAR szSrvIP[BUFFER_SIZE] = {0};
	strcpy(szSrvIP, argv[1]);
	HUINT16 usSrvPort = atoi(argv[2]);

	HDSMClient *pCli = new HDSMClient(szSrvIP, usSrvPort);
	if (pCli == NULL)
	{
		cout<<"Create client Failed!"<<endl;
		return -1;
	}

	if (!pCli->is_valid())
	{
		cout<<"Connect to HDSM server Failed!"<<endl;
		return -1;
	}

	cout <<"Welcome to HDSM!"<< endl << ">";
	

	HDSMCommandExecutor executor(pCli);

	HCHAR szCMD[BUFFER_SIZE] = {0};
	while (fgets(szCMD, BUFFER_SIZE-1, stdin) != NULL)
	{
		if (!executor.exec(szCMD))
		{
			//cout << "Invalid Client or Command!" << endl;
			print_command_help_info();
			cout << ">";
			memset(szCMD, 0, BUFFER_SIZE);
			continue;
		}

		if (executor.get_cur_command_op() == OP_QUIT)
		{
			cout << "The client is closed!" << endl;
			break;
		}

		cout << executor.get_cur_format_result_info().c_str() << endl;

		cout << ">";
		memset(szCMD, 0, BUFFER_SIZE);
	}

	cout <<"Press any key to exit!" <<endl;
	getchar();

	if (pCli != NULL)
		delete pCli;

	return 0;
}

