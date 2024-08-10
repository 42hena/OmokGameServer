#include <process.h>

#include <map>
#include <unordered_map>
#include <Windows.h>



#include "Job.h"
#include "CPacket.h"
#include "CRBuffer.h"
#include "CLFQueue.h"
#include "CLFStack.h"
#include "Session.h"
#include "CBucketPool.h"

#include "Player.h"

#include "CGroup.h"
#include "CGroupManager.h"
#include "CNetLibrary.h"
#include "CNetworkClientLib.h"
#include "CMonitorChatClient.h"
#include "CGameServerInterface.h"

#include <Pdh.h>
#include <Windows.h>
#include "CPDH.h"


CGameServerInterface::CGameServerInterface()
	: playerPool{15000, true}
{
	InitializeSRWLock(&playerMapLock);
	monitorClientPtr = new CMonitorChatClient(2);

	hMonitorThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
	if (hMonitorThread == nullptr)
	{
		DebugBreak();
		return;
	}
}

//CGameServerInterface::CGameServerInterface(void *ptr)
//	: playerPool{ 15000, true }
//{
//	InitializeSRWLock(&playerMapLock);
//}

CGameServerInterface::~CGameServerInterface()
{
	
}

bool CGameServerInterface::OnConnectionRequest(const WCHAR* IPAddress, USHORT port)
{
	return true;
}

void CGameServerInterface::OnClientJoin(unsigned __int64 sessionId)
{
	SPlayer* newPlayer;
	// -----

	newPlayer = playerPool.Alloc();
	newPlayer->sessionId = sessionId;
	AcquireSRWLockExclusive(&playerMapLock);
	playersMap.insert({ sessionId, newPlayer });
	ReleaseSRWLockExclusive(&playerMapLock);
}

void CGameServerInterface::OnClientLeave(unsigned __int64 sessionID)
{
	SPlayer* player;


	AcquireSRWLockExclusive(&playerMapLock);
	auto it = playersMap.find(sessionID);
	player = it->second;
	long s = playersMap.erase(sessionID);
	if (s == 0)
		DebugBreak();

	/*s = playersKey.erase(player->accountNo);
	if (s == 0)
		DebugBreak();*/

	ReleaseSRWLockExclusive(&playerMapLock);
	playerPool.Free(player);
}

void CGameServerInterface::OnError(int errorcode, const WCHAR* msg)
{

}



SPlayer* CGameServerInterface::FindPlayer(unsigned __int64 sessionId)
{
	AcquireSRWLockShared(&playerMapLock);
	auto it = playersMap.find(sessionId);
	ReleaseSRWLockShared(&playerMapLock);
	if (it == playersMap.end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}
}
#include "CCpuUsage.h"

unsigned int CGameServerInterface::MonitorThread(LPVOID param)
{
	CGameServerInterface* thisPtr = reinterpret_cast<CGameServerInterface*>(param);
	CCpuUsage cpuUsage;
	// -----

	static DWORD firstTime;
	DWORD lastTime;
	DWORD sec, min, hour;

	long packetPoolCnt;
	int total;
	char filename[256];
	// start
	firstTime = timeGetTime();

	FILE* fp;
	time_t currentTime;
	struct tm localTime;


	CProcessPDH processPdh(L"CGameServer");

	long threadCount, handleCount;
	long userCPU, useMem;// 11, 12

	long totalSessionCount, currentSessionCount; //13
	long useIndexCount;


	long authPlayerCount;// 14
	long fieldPlayerCount; // 15
	long acceptTPS;	// 16
	long recvTPS;	// 17
	long sendTPS;	// 18
	long db1, db2;	// 19, 20
	long lobbyFPS;	// 21
	long fieldFPS;	// 22

	long totalBucket, useBucket, totalNode, useNode; //23

	for (; ; )
	{
		currentTime = time(NULL);
		localtime_s(&localTime, &currentTime);

		sprintf_s(filename, sizeof(filename), "file_%04d-%02d-%02d_%02d.txt",
			localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
			localTime.tm_hour);

		lastTime = timeGetTime();
		total = (lastTime - firstTime) / 1000;
		sec = total % 60;
		total /= 60;
		min = total % 60;
		total /= 60;
		hour = total;
		cpuUsage.UpdateCpuTime();
		lobbyFPS = thisPtr->manager.lobby.GetFrame();
		fieldFPS = thisPtr->manager.field.GetFrame();

		//flag, cpu, mem, sessionCnt, playerCnt, updateTPS, packetPoolCnt, JobPoolCnt;
		currentSessionCount = thisPtr->GetCurrentSessionCount();

		threadCount = processPdh.GetThreadCount();
		handleCount = processPdh.GetHandleCount();
		//userCPU = processPdh.GetUserCpuInteger();
		userCPU = cpuUsage.ProcessUser();
		useMem = processPdh.GetPrivateMem() / 1024 / 1024;

		acceptTPS = thisPtr->GetAcceptTPS();
		recvTPS = thisPtr->GetRecvMessageTPS();
		sendTPS = thisPtr->GetSendMessageTPS();
		// authTPS = thisPtr->GetAuthTPS();

		totalSessionCount = thisPtr->GetSessionCount();
		useIndexCount = thisPtr->GetIndexPoolUseSize();


		//thisPtr->manager
		totalBucket = CPacket::GetTotalBucket();
		useBucket = CPacket::GetUseBucket();
		totalNode = CPacket::GetTotalNode();
		useNode = CPacket::GetUseNode();

		authPlayerCount = thisPtr->manager.lobby.groupPlayer.size();
		fieldPlayerCount = thisPtr->manager.field.groupPlayer.size();

		wprintf(L"----- Running Time %3d h - %2d m - %2d s -----\n", hour, min, sec);
		wprintf(L"[Process]\n");
		//wprintf(L"Connect:%d\n", thisPtr->monitorClientPtr->loginOnFlag);
		wprintf(L"%-15s %10d | %-15s %10d |\n",
			L"Thread count:", threadCount,
			L"Handle count :", handleCount);
		wprintf(L"%-15s %10d | %-15s %10d |\n",
			L"CPU:", userCPU,
			L"Mem:", useMem);
		wprintf(L"%-15s %10d | %-15s %10d |\n",
			L"Lobby:", lobbyFPS,
			L"Field:", fieldFPS);
		wprintf(L"[Network]\n");

		wprintf(L"%-15s %10d | %-15s %10d | %-15s %10d |\n",
			L"AcceptTPS:", acceptTPS,
			L"RecvTPS:", recvTPS,
			L"SendTPS:", sendTPS);
		wprintf(L"[Info]\n");
		wprintf(L"%-15s %10d | %-15s %10d | %-15s %10d |\n",
			L"TotalSession:", totalSessionCount,
			L"CurrentSession:", currentSessionCount,
			L"Use Index:", useIndexCount);
		wprintf(L"%-15s %10d | %-15s %10d | %-15s %10d | %-15s %10d |\n",
			L"GetTotalBucket:", totalBucket,
			L"GetUseBucket:", useBucket,
			L"GetTotalNode:", totalNode,
			L"GetUseNode:", useNode);
		wprintf(L"%-15s %10d | %-15s %10d | %-15s %10d | %-15s %10d |\n",
			L"Lobby session:", thisPtr->manager.lobby.indexSize(),
			L"Lobby player:", thisPtr->manager.lobby.groupPlayer.size(),
			L"Field session:", thisPtr->manager.field.indexSize(),
			L"Field player:", thisPtr->manager.field.groupPlayer.size());

		wprintf(L"%-15s %10d | %-15s %10d | %-15s %10d|\n",
			L"Error(=):", testArrayIdx,
			L"Error2:", testArrayIdx2,
			L"Error3:", testArrayIdx3);

		wprintf(L"--------------------------------------------------\n\n");

		if (thisPtr->monitorClientPtr->gameMonitorFlag)
		{
			thisPtr->monitorClientPtr->gameMonitorFlag = thisPtr->monitorClientPtr->gameMonitorFlag;
			thisPtr->monitorClientPtr->gameServerCpu = userCPU;
			thisPtr->monitorClientPtr->gameServerMem = useMem;
			thisPtr->monitorClientPtr->gameSessionCount = currentSessionCount;
			thisPtr->monitorClientPtr->gameAuthPlayerCount = authPlayerCount; //Auth
			thisPtr->monitorClientPtr->gameFieldPlayerCount = fieldPlayerCount; //player
			thisPtr->monitorClientPtr->gameAcceptTPS = acceptTPS;
			thisPtr->monitorClientPtr->gameRecvTPS = recvTPS;
			thisPtr->monitorClientPtr->gameSendTPS = sendTPS;
			thisPtr->monitorClientPtr->gameDBWriteTPS = 0;
			thisPtr->monitorClientPtr->gameDBMSGPacketCount = 0;
			thisPtr->monitorClientPtr->gameLobbyFPS = lobbyFPS;
			thisPtr->monitorClientPtr->gameFieldFPS = fieldFPS;
			thisPtr->monitorClientPtr->gamePacketUseTotalCount = useNode;
			SetEvent(thisPtr->monitorClientPtr->hJobEvent);
		}

		Sleep(999);
	}

	return (0);
}


void CGameServerInterface::SRWELock()
{
	AcquireSRWLockExclusive(&playerMapLock);
}

void CGameServerInterface::SRWEUnLock()
{
	ReleaseSRWLockExclusive(&playerMapLock);
}

void CGameServerInterface::SRWSLock()
{
	AcquireSRWLockShared(&playerMapLock);
}

void CGameServerInterface::SRWSUnLock()
{
	ReleaseSRWLockShared(&playerMapLock);
}

void CGameServerInterface::ConnectToMonitorServer()
{
	if (!monitorClientPtr->flag)
	{
		monitorClientPtr->TryToConnectServer(L"127.0.0.1", 12345, 1, true);
	}
	else
		monitorClientPtr->RetryToConnectServer();
}
