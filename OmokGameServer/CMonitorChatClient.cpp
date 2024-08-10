#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <process.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include <unordered_map>


#include "CRBuffer.h"
#include "CLFQueue.h"

#include "CPacket.h"
#include "Job.h"
#include "Session.h"
#include "CNetworkClientLib.h"
#include "CMonitorChatClient.h"


CMonitorChatClient::CMonitorChatClient(int serverNo)
	: serverNo(serverNo),
	chatOnFlag(0),
	chatCPUUsage(0),
	chatMemUsage(0),
	chatSessionCount(0),
	chatPlayerCount(0),
	chatUpdateTPS(0),
	chatPacketPoolUsage(0),
	chatJobPoolUsage(0)
{

	hJobEvent = CreateEvent(nullptr, false, 0, nullptr);
	if (hJobEvent == nullptr)
	{
		DebugBreak();
	}
	wprintf(L"CMonitorChatClient constructor\n");


	// TODO:03-28
	HANDLE sss = (HANDLE)_beginthreadex(nullptr, 0, SendThread, this, 0, nullptr);
	if (sss == nullptr)
	{
		DebugBreak();
	}

}

CMonitorChatClient::~CMonitorChatClient()
{
	wprintf(L"CMonitorChatClient destructor\n");
}

void CMonitorChatClient::OnEnterJoinServer()
{
	chatOnFlag = 1;
	wprintf(L"OnEnterJoinServer\n");
}

void CMonitorChatClient::OnLeaveServer()
{
	chatOnFlag = 0;
	wprintf(L"OnLeaveServer\n");
}

void CMonitorChatClient::OnMessage(CPacket* packetPtr)
{
	WORD type;
	//-----

	*packetPtr >> type;
	switch (type)
	{
	default:
		break;
	}

}

void CMonitorChatClient::OnError(int errorcode, const wchar_t* msg)
{
	wprintf(L"msg:%s\n", msg);
}

void CMonitorChatClient::OnThreadExit(const wchar_t* msg)
{
	wprintf(L"msg:%s\n", msg);
}

void CMonitorChatClient::CreateMonitorLoginPacket(CPacket* packetPtr)
{
	const WORD type = en_PACKET_SS_MONITOR_LOGIN;
// -----

	*packetPtr << type << serverNo;
}

//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
//		int		DataValue				// 해당 데이터 수치.
//		int		TimeStamp
void CMonitorChatClient::CreateDataUpdatePacket(CPacket* packetPtr, BYTE dataType)
{
	const WORD type = en_PACKET_SS_MONITOR_DATA_UPDATE;
	time_t timeStamp;
// -----

	*packetPtr << type << dataType;

	switch (dataType)
	{
	case 30:
		*packetPtr << chatOnFlag;
		break;
	case 31:
		*packetPtr << chatCPUUsage;
		wprintf(L"Test:%d\n", chatCPUUsage);

		break;
	case 32:
		*packetPtr << chatMemUsage;
		break;
	case 33:
		*packetPtr << chatSessionCount;
		break;
	case 34:
		*packetPtr << chatPlayerCount;
		break;
	case 35:
		*packetPtr << chatUpdateTPS;
		break;
	case 36:
		*packetPtr << chatPacketPoolUsage;
		break;
	case 37:
		*packetPtr << chatJobPoolUsage;
		break;
	default:
		DebugBreak();
		break;
	}

	time(&timeStamp);
	*packetPtr << (int)timeStamp;
}

void CMonitorChatClient::SendChatInfo()
{
	CPacket* packetPtr;
	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 30);
	SendMessages(packetPtr);
	packetPtr->subRef();


	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 31);
	SendMessages(packetPtr);
	packetPtr->subRef();

	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 32);
	SendMessages(packetPtr);
	packetPtr->subRef();

	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 33);
	SendMessages(packetPtr);
	packetPtr->subRef();

	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 34);
	SendMessages(packetPtr);
	packetPtr->subRef();

	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 35);
	SendMessages(packetPtr);
	packetPtr->subRef();

	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 36);
	SendMessages(packetPtr);
	packetPtr->subRef();

	packetPtr = CPacket::Alloc();
	packetPtr->AddRef();
	CreateDataUpdatePacket(packetPtr, 37);
	SendMessages(packetPtr);
	packetPtr->subRef();


	/*packetPtr = CPacket::Alloc();
	packetPtr->AddRef();

	WORD type = 20000;
	long data = 1;
	*packetPtr << type << data;
	SendMessages(packetPtr);
	packetPtr->subRef();*/
}

unsigned int CMonitorChatClient::SendThread(LPVOID param)
{
	const int handleCount = 2;
	HANDLE handleArrays[handleCount];
	CMonitorChatClient* thisPtr;
	int code;
// -----
	thisPtr = reinterpret_cast<CMonitorChatClient*>(param);

	handleArrays[0] = thisPtr->GetExitEvent();
	handleArrays[1] = thisPtr->GetJobEvent();

	while (1)
	{
		code = WaitForMultipleObjects(handleCount, handleArrays, false, INFINITE);
		if (code == WAIT_OBJECT_0)
		{
			break;
		}
		else if (code == WAIT_OBJECT_0 + 1)
		{
			wprintf(L"SendThread\n");
			thisPtr->SendChatInfo();
		}
		else
		{
			DebugBreak();
		}
	}

	return 0;
}

HANDLE CMonitorChatClient::GetJobEvent()
{
	return ( hJobEvent );
}