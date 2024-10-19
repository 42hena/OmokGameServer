#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <winsock2.h>
#include <process.h>
#include <list>
#include <set>
#include <conio.h>
#include <unordered_map>
#include <string>
#include <Windows.h>

#include "CLFQueue.h"
#include "CLFStack.h"
#include "CRBuffer.h"

#include "CUser.h"
#include "CRoom.h"

#include "CPacket.h"

#include "Session.h"
#include "Job.h"

#include "CNetLibrary.h"
#include "CServer.h"


CServer server;

void RunServer()
{
	char keyboard;
	bool keyFlag;
	keyFlag = 0;
	wprintf(L"Omok Game Server\n");
	server.ExcuteServer(L"0.0.0.0", 12001, 2, 1, false, 1000);

	for (; ; )
	{
		if (_kbhit())
		{
			keyboard = _getch();
			/*		
			if (keyboard == 'z')
			{
				keyFlag = !keyFlag;
				wprintf(L"Mode Change\n");
			}
			if (keyFlag && keyboard == 's')
			{
				wprintf(L"Save\n");
			}
			if (keyFlag && keyboard == 'l')
			{

			}

			if (keyFlag && keyboard == 'm')
			{
				server.ConnectToMonitorServer();
			}*/
		}
		Sleep(1000);
	}
}

int main()
{
	timeBeginPeriod(1);

	RunServer();

	timeEndPeriod(1);
	return (0);
}