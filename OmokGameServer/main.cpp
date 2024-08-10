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

#include "CNetworkClientLib.h"
#include "CMonitorChatClient.h"
#include "CNetLibrary.h"
#include "CServer.h"


CServer server;

int main()
{
	char keyboard;
	bool keyFlag;
// -----

	timeBeginPeriod(1);

	// 파일 읽기 등 초기화 작업이 필요.
	// .ini 파일을 읽는 느낌으로 가야할 듯.

	keyFlag = 0;
	wprintf(L"Omok Game Server\n");
	server.ExcuteServer(L"0.0.0.0", 12001, 2, 1, false, 1000);

	for( ; ; )
	{
		if (_kbhit())
		{
			keyboard = _getch();
	/*		if (keyboard == 'z')
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
	
	timeEndPeriod(1);
	return (0);
}