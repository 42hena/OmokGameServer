#include <iostream>
#include <process.h>

#include <unordered_map>
#include <list>
#include <string>
#include <set>

#include <WinSock2.h>
#include <Pdh.h>
#include <Windows.h>

#include "CRBuffer.h"
#include "CLFStack.h"
#include "CLFQueue.h"

#include "CUser.h"
#include "CRoom.h"

#include "CPacket.h"
#include "Job.h"

#include "ChatProtocol.h"
#include "Session.h"

//#include "CNetworkClientLib.h"
//#include "CMonitorChatClient.h"

#include "CNetLibrary.h"
#include "CServer.h"

#include "CPDH.h"
#include "Protocol.h"

CLFQueue<SJob> jobQ(500'001);

std::unordered_map<unsigned __int64, CUser*> userManager;
std::unordered_map<uintptr_t, uintptr_t> accountKey;

alignas (64) uintptr_t messageRecvCount = 0;
alignas (64) uintptr_t messageSendCount = 0;
alignas (64) uintptr_t loginPacket = 0;
alignas (64) uintptr_t messagePacket = 0;
alignas (64) uintptr_t sectorPacket = 0;

// CServer
CServer::CServer()
	: userCount(0), updateCount(0)
{
	int errCode;
	int idx;
	HANDLE hUpdateThread;
	HANDLE hMonitorThread;
	// -----

	// TODO: 03-26
	//monitorClientPtr = new CMonitorChatClient(1);

	wprintf(L"CServer Constructor\n");
	hUpdateThread = (HANDLE)_beginthreadex(nullptr, 0, UpdateThread, this, 0, nullptr);
	if (hUpdateThread == nullptr)
	{
		errCode = GetLastError();
		wprintf(L"hUpdateThread Fail %d\n", errCode);
		return;
	}
	/*hMonitorThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
	if (hMonitorThread == nullptr)
	{
		errCode = GetLastError();
		wprintf(L"hMonitorThread Fail %d\n", errCode);
		return;
	}*/
}

CServer::~CServer()
{
	wprintf(L"CServer Destructor\n");
}

// 유저 생성만 하고, 로그인 패킷 시 다시 채우기.
void CServer::CreateUser(unsigned __int64 id)
{
	CUser* newUser;
// -----

	wprintf(L"CreateUser\n");
	newUser = userPool.Alloc();


	newUser->_sessionId = id;
	userManager.insert({ id, newUser });

	InterlockedIncrement(&userCount);
}

CUser* CServer::FindUser(unsigned __int64 sID)
{
	auto iter = userManager.find(sID);
	if (iter == userManager.end())
	{
		DebugBreak();
		return (nullptr);// 중복
	}
	return iter->second;
}

void CServer::ReleaseUser(unsigned __int64 sID)
{
	CUser* pUser;
	//-----

	pUser = FindUser(sID);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}


	size_t size = userManager.erase(sID);
	if (size == 0)
		DebugBreak();

	WORD roomNo = pUser->GetCurrentRoom();
	auto roomIt = _roomManager.find(roomNo);
	if (roomIt == _roomManager.end())
	{
		;
	}
	else
	{
		CChatRoom* pRoom = roomIt->second;
		uintptr_t accountNo = pUser->GetCurrentAccountNo();
		pRoom->EraseUser(accountNo);
		int ss = accountKey.erase(accountNo);
		if (ss == 0)
			DebugBreak();

	}

	InterlockedDecrement(&userCount);
	userPool.Free(pUser);
}

unsigned int CServer::UpdateThread(LPVOID param)
{
	CServer* ptr = reinterpret_cast<CServer*>(param);
	DWORD WaitForSingleObjectRet;
	SJob job;
	int jobQRet;
// -----
	wprintf(L"UpdateThread Start\n");
	for (;;)
	{
		WaitForSingleObjectRet = WaitForSingleObject(ptr->GetJobEvent(), INFINITE);
		if (WaitForSingleObjectRet == WAIT_OBJECT_0)
		{
			while (1)
			{
				jobQRet = jobQ.Dequeue(job);
				if (jobQRet == -1) // JobQ 다 빠짐.
					break;
				InterlockedIncrement(&ptr->updateCount);
				switch (job.type)
				{
				case en_JOB_TYPE::en_JobOnAccept:
				{
					ptr->CreateUser(job.sessionId);
					break;
				}
				case en_JOB_TYPE::en_JobOnMessage:
				{
					ptr->Recv(job.sessionId, job.data);
					break;
				}
				case en_JOB_TYPE::en_JobOnRelease:
				{
					ptr->ReleaseUser(job.sessionId);
					break;
				}
				case en_JOB_TYPE::en_JobServerDown:
					return (0);
					break;
				case en_JOB_TYPE::en_JobSystem: // 시스템 관련
					break;
				default:
					DebugBreak(); // TODO????
					break;
				}
			}

		}
		else if (WaitForSingleObjectRet == WAIT_ABANDONED || WaitForSingleObjectRet == WAIT_TIMEOUT || WaitForSingleObjectRet == WAIT_FAILED)
		{
			wprintf(L"UpdateThread %d\n", WaitForSingleObjectRet);
		}
		else
		{
			wprintf(L"UpdateThread Crashed\n");
		}
	}

	return (0);
}


//unsigned int CServer::MonitorThread(LPVOID param)
//{
//	CServer* ptr = reinterpret_cast<CServer*>(param);
//// -----
//
//	static DWORD firstTime;
//	DWORD lastTime;
//	DWORD sec, min, hour;
//
//	long flag, cpu, mem, sessionCnt, playerCnt, updateTPS, packetPoolCnt, JobPoolCnt;
//	int total;
//	char filename[256];
//	// start
//	firstTime = timeGetTime();
//	
//	FILE* fp;
//	time_t currentTime;
//	struct tm localTime;
//
//	
//	CProcessPDH processPdh(L"CNetworkLib");
//
//
//	for ( ; ; )
//	{
//		currentTime = time(NULL);
//		localtime_s(&localTime, &currentTime);
//		
//		sprintf_s(filename, sizeof(filename), "file_%04d-%02d-%02d_%02d.txt",
//			localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
//			localTime.tm_hour);
//		/*fopen_s(&fp, filename, "a+");
//		if (fp == nullptr)
//		{
//			DebugBreak();
//			return 1;
//		}*/
//		lastTime = timeGetTime();
//		total = (lastTime - firstTime) / 1000;
//		sec = total % 60;
//		total /= 60;
//		min = total % 60;
//		total /= 60;
//		hour = total % 60;
//		
//		total = 0;
//		
//		flag = 0;
//		// CPU Mem 줄 cpu, mem
//		
//		for (int i = 0; i < 50; ++i)
//		{
//			for (int j = 0; j < 50; ++j)
//			{
//				total += secterList[i][j].size();
//			}
//		}
//
//		//flag, cpu, mem, sessionCnt, playerCnt, updateTPS, packetPoolCnt, JobPoolCnt;
//		sessionCnt = ptr->GetUser();
//		playerCnt = accountManager.size();// 삭제
//
//		updateTPS = ptr->GetMonitoringUpdateCount();
//		packetPoolCnt = CPacket::GetUseNode();
//		JobPoolCnt = jobQ.GetSize();
//
//
//
//		wprintf(L"Running Time:%3d h - %2d m - %2d s | server Status %d\n", hour, min, sec, flag);
//		wprintf(L"Monitor: %d\n", flag);
//		wprintf(L"Thread count: %d\n", processPdh.GetThreadCount());
//		wprintf(L"--------------------------------------------------\n");
//		wprintf(L"TotalSession:%d user:%d account:%llu sector:%d avg:%lf\n", ptr->GetSessionCount(), sessionCnt, accountManager.size(), total, (float)total / 2500 * 9);// manager 삭제
//		wprintf(L"indexPool:%d characterPool:%d\n", ptr->GetIndexPoolUseSize(), ptr->GetUserPoolUseSize());
//		wprintf(L"Accept:%d Recv:%d Send:%d\n", ptr->GetAcceptTPS(), ptr->GetRecvMessageTPS(), ptr->GetSendMessageTPS());
//		wprintf(L"Login:%llu Sector:%llu Message Recv:%llu Send:%llu\n", InterlockedExchange(&loginPacket, 0), InterlockedExchange(&sectorPacket, 0), InterlockedExchange(&messageRecvCount, 0), InterlockedExchange(&messageSendCount, 0));
//		wprintf(L"CPacket GetTotalBucket:%d GetUseBucket:%d  GetTotalNode:%d GetUseNode:%d\n", CPacket::GetTotalBucket(), CPacket::GetUseBucket(), CPacket::GetTotalNode(), CPacket::GetUseNode());
//		wprintf(L"JobQ Total:%d Pool:%d use:%d\n", jobQ.GetPoolAllocSize(), jobQ.GetPoolUseSize(), jobQ.GetSize());
//		wprintf(L"Update:%d\n\n", updateTPS);
//
//		
//
//		/*fprintf(fp, "This is a test.\n");
//		fprintf(fp, "Current time: %d-%02d-%02d %02d:%02d:%02d\n",
//			localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
//			localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
//		fclose(fp);*/
//		if (ptr->monitorClientPtr->chatOnFlag)
//		{
//			// ptr->monitorClientPtr->chatOnFlag = flag;
//			ptr->monitorClientPtr->chatCPUUsage = processPdh.GetUserCpuInteger();
//			ptr->monitorClientPtr->chatMemUsage = processPdh.GetPrivateMem()/1024/1024;
//			ptr->monitorClientPtr->chatSessionCount = sessionCnt;
//			ptr->monitorClientPtr->chatPlayerCount = playerCnt;
//			ptr->monitorClientPtr->chatUpdateTPS = updateTPS;
//			ptr->monitorClientPtr->chatPacketPoolUsage = packetPoolCnt;
//			ptr->monitorClientPtr->chatJobPoolUsage = JobPoolCnt;
//
//			// Wake SendThread
//			SetEvent(ptr->monitorClientPtr->GetJobEvent());
//		}
//		Sleep(999);
//	}
//
//	/*long chatOnFlag;
//	long chatCPUUsage;
//	long chatMemUsage;
//	long chatSessionCount;
//	long chatPlayerCount;
//	long chatUpdateTPS;
//	long chatPacketPoolUsage;
//	long chatJobPoolUsage;*/
//
//	/*int total = 0;
//	for (int i = 0; i < 50; ++i)
//	{
//		for (int j = 0; j < 50; ++j)
//		{
//			total += secterList[i][j].size();
//		}
//	}
//
//	CProfiler s(L"Monitor");
//	wprintf(L"TotalSession:%d user:%d account:%llu sector:%llu avg:%lf\n", server.GetSessionCount(), server.GetUser(), accountManager.size(), total, (float)total / 2500 * 9);
//	wprintf(L"indexPool:%d characterPool:%d\n", server.GetIndexPoolUseSize(), server.GetUserPoolUseSize());
//	wprintf(L"Accept:%d Recv:%d Send:%d\n", server.GetAcceptTPS(), server.GetRecvMessageTPS(), server.GetSendMessageTPS());
//	wprintf(L"Login:%llu Sector:%llu Message Recv:%llu Send:%llu\n", InterlockedExchange(&loginPacket, 0), InterlockedExchange(&sectorPacket, 0), InterlockedExchange(&messageRecvCount, 0), InterlockedExchange(&messageSendCount, 0));
//	wprintf(L"CPacket GetTotalBucket:%d GetUseBucket:%d  GetTotalNode:%d GetUseNode:%d\n", CPacket::GetTotalBucket(), CPacket::GetUseBucket(), CPacket::GetTotalNode(), CPacket::GetUseNode());
//	wprintf(L"JobQ Total:%d Pool:%d use:%d\n", jobQ.GetPoolAllocSize(), jobQ.GetPoolUseSize(), jobQ.GetSize());
//	wprintf(L"Update:%d\n\n", server.GetMonitoringUpdateCount());
//	Sleep(999);*/
//
//	return (0);
//}

bool CServer::OnConnectionRequest(const WCHAR* IPAddress, USHORT port)
{
	wprintf(L"OnConnectionRequest[IP:%s Port:%d]\n", IPAddress, port);

	// black IP의 경우 false

	// (특정 키 눌려 있을 경우)
	//	white IP ok
	// else
	// false

	return (true);
}

void CServer::OnClientJoin(unsigned __int64 sessionID)
{
	SJob newJob;
	BOOL SetEventRet;
	// -----

	newJob.type = en_JOB_TYPE::en_JobOnAccept;
	newJob.sessionId = sessionID;
	newJob.data = nullptr;

	// jobQ에 일 넣기.
	jobQ.Enqueue(newJob);

	// Update Thread 깨우기
	SetEventRet = SetEvent(GetJobEvent());
	if (SetEventRet == 0)
	{
		DebugBreak();
	}
}

void CServer::OnClientLeave(unsigned __int64 sessionID)
{
	SJob newJob;
	BOOL SetEventRet;
	// -----

	newJob.type = en_JOB_TYPE::en_JobOnRelease;
	newJob.sessionId = sessionID;
	newJob.data = nullptr;

	// jobQ에 일 넣기.
	jobQ.Enqueue(newJob);

	// Update Thread 깨우기
	SetEventRet = SetEvent(GetJobEvent());
	if (SetEventRet == 0)
	{
		DebugBreak();
	}
}

void CServer::OnMessage(unsigned __int64 sessionID, CPacket* packet)
{
	SJob newJob;
	BOOL SetEventRet;
	// -----

	// TODO: job 초기화 함수 만들기.
	newJob.type = en_JOB_TYPE::en_JobOnMessage;
	newJob.sessionId = sessionID;
	newJob.data = packet;

	// jobQ에 일 넣기.
	jobQ.Enqueue(newJob);

	// Update Thread 깨우기
	SetEventRet = SetEvent(GetJobEvent());
	if (SetEventRet == 0)
	{
		DebugBreak();
	}
}

void CServer::OnError(int errorcode, const WCHAR* msg)
{
	/*if (errorcode == 0)
	{
		wprintf(L"hi %s\n", msg);
	}*/
}

int CServer::LoginTest(unsigned __int64 sID, CPacket* packet)
{
	CUser* user;

	INT64 accountNo;
	BYTE nickLen;
	CHAR nickName[20];
	//char sessionKey[64];
	//std::wstring nickName;
	// -----

	user = FindUser(sID);
	if (user == nullptr)
	{
		DebugBreak();
		packet->subRef();
		return false;
	}
	// packet 값 빼기.
	*packet >> accountNo;

	// 이미 접속.
	auto it = accountKey.find(accountNo);
	if (it != accountKey.end())
	{
		SSession* s = FindSession(it->second);
		wprintf(L"IP:%s port:%d\n", s->clientIP, s->port);
		wprintf(L"-----------------------------------------------------\n");
		wprintf(L"-----------------------------------------------------\n");
		wprintf(L"-----------------------------------------------------\n");
		
		//1 방금 접속한 사람 끊기
		Disconnect(sID);
		DebugBreak();
		return false;
	}

	*packet >> nickLen;
	packet->GetData((char*)nickName, nickLen);
	//packet->GetData((char*)sessionKey, sizeof(sessionKey));

	// user member variable 초기화
	user->_accountNo = accountNo;
	//memcpy(user->id, id, sizeof(id));
	//user->_nickName = nickName; null 필요
	user->_nickName.assign(nickName, strlen(nickName));

	//memcpy((char*) &(user->_nickName).c_str(), nickName, sizeof(nickName));
	//memcpy(user->sessionKey, sessionKey, sizeof(sessionKey));

	accountKey.insert({ accountNo, sID });


	packet->Clear();

	// Create sendPacket 
	WORD type;
	BYTE status;
	
	type = en_PACKET_CS_CHAT_RES_LOGIN;
	packet->PutData((char*)&type, sizeof(type));
	status = 1;
	packet->PutData((char*)&accountNo, sizeof(accountNo));
	packet->PutData((char*)&status, sizeof(status));

	// type(2) | accountNo(8) status(1)

	// sendPacket 전송 요청
	SendMessages(sID, packet);
	packet->subRef();

	return (true);
}

void CServer::Recv(unsigned __int64 sID, CPacket* packet)
{
	WORD type;
// -----

	if (packet == nullptr)
	{
		DebugBreak();
	}

	*packet >> type;

	switch (type)
	{
	case en_LoginRequest: // Login Req
	{
		LoginTest(sID, packet);
		break;
	}
	case en_HeartBeatRequest: // 하트비트
	{
		// HeartBeat(sID, packet);
		break;
	}
	case en_RoomListRequest:	// RoomList 요청
	{
		RoomList(sID, packet);
		break;
	}
	case en_CreateRoomRequest:	// Room 생성
	{
		CreateNewRoom(sID, packet);
		break;
	}
	case en_EnterRoomRequest:
	{
		EnterRoomResponse(sID, packet);
		break;
	}
	case en_LeaveRoomRequest:
	{
		LeaveRoomResponse(sID, packet);
		break;
	}
	case en_ChatRequest:
	{
		ChatRoom(sID, packet);
		break;
	}
	case en_ChangePositionPlayerRequest:
		ChangePosition(sID, packet);
		break;
	case en_ChangePositionSpectatorRequest:
		ChangePosition(sID, packet);
		break;

	case en_ReadyRequest:
	{
		Ready(sID, packet);
		break;
	}
	case en_CancelReadyRequest:
	{
		CancelReady(sID, packet);
		break;
	}
	case en_PutStoneRequest:
	{
		PutStone(sID, packet);
		break;
	}

	default:
		Disconnect(sID); // Attack
		packet->subRef();
		break;
	}
}

void CServer::RoomList(unsigned __int64 sID, CPacket* pRecvPacket)
{
	uintptr_t accountNo;
	USHORT index;
	// -----

	// len | type | len, name[]

	*pRecvPacket >> accountNo >> index;


	// CPacket 가공

	CPacket* pResponsePacket;
	const USHORT type = en_RoomListResponse;
	pResponsePacket = pRecvPacket;



	// len | type(2) | accountNo(8) roomCount(2), [num(2), roomLen(1) roomName(roomLen)]
	pResponsePacket->Clear();
	WORD roomCount = _roomManager.size();


	int r = roomCount % 40;
	int q = roomCount / 40;
	if (index < q)
	{
		roomCount = 40;
	}
	else if (index == q)
	{
		roomCount = r;
	}
	else
		roomCount = 0;
	*pResponsePacket << type << accountNo << roomCount;

	int cnt = 0;
	for (auto it = _roomManager.begin(); it != _roomManager.end(); ++it)
	{
		// TODO
		if (cnt >= 40 * (index) && cnt < 40 * (index + 1))
		{
			CChatRoom* pRoom = it->second;
			WORD lll = pRoom->GetCurrentRoomNumber();
			*pResponsePacket << lll;
			BYTE ss = pRoom->GetCurrentRoomName().size();
			*pResponsePacket << ss;
			pResponsePacket->PutData((char*)pRoom->GetCurrentRoomName().c_str(), ss);
		}
		cnt++;
	}
	SendMessages(sID, pResponsePacket);

	pResponsePacket->subRef();
}

void CServer::CreateNewRoom(unsigned __int64 sID, CPacket* pRecvPacket)
{
	uintptr_t accountNo;
	static USHORT roomNum = 0;
	BYTE roomNameLength;
	char temp[20];
	// -----

	// len | type | len, name[]

	*pRecvPacket >> accountNo >> roomNameLength;
	pRecvPacket->GetData((char *)& temp, roomNameLength);
	temp[roomNameLength] = 0;

	std::string roomName(temp);

	CChatRoom* pRoom = roomPool.Alloc();
	pRoom->ChangeRoomName(roomName);

	while (1)
	{
		++roomNum;
		if (roomNum == 0)
			continue;

		auto roomIt = _roomManager.find(roomNum);
		if (roomIt == _roomManager.end())
		{
			break;
		}
	}
		
	pRoom->ChangeRoomNumber(roomNum);

	// 검증
	

	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
	{
		DebugBreak();
	}

	CUser* pUser = userIt->second;
	
	pUser->Logging(en_CreateRoomRequest, accountNo, roomNum, pUser->_state);

	_roomManager.insert({ roomNum , pRoom });
	pRoom->AddUser( accountNo, pUser );
	pUser->_inRoom = roomNum;
	pUser->ChangePositionSpectator();
	/*auto userIt = userManager.find(sID);

	pRoom->AddUser(accountNo, userIt->second);*/
	// CPacket 가공

	CPacket* pResponsePacket;
	WORD type = en_CreateRoomResponse;
	pResponsePacket = pRecvPacket;
	
	BYTE flag = 1;
	WORD roomId = roomNum;
	
	// len | type(2) | flag(1), len(1), room(len) = 9 + len
	pResponsePacket->Clear();
	*pResponsePacket << type << flag << roomId << roomNameLength;
	pResponsePacket->PutData((char*)roomName.c_str(), roomNameLength);

	SendMessages(sID, pResponsePacket);

	pResponsePacket->subRef();
}

void CServer::EnterRoomResponse(unsigned __int64 sID, CPacket* pRecvPacket)
{
	uintptr_t accountNo;
	WORD roomNum;
	// -----
	// len | type ||| flag roomid roomLen roomName

	*pRecvPacket >> accountNo >> roomNum;
	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
		DebugBreak();
	CUser* pUser = userIt->second;
	CPacket* pResponsePacket;

	pResponsePacket = pRecvPacket;
	pResponsePacket->Clear();
	BYTE flag = 1;
	WORD type = en_EnterRoomResponse;
	CChatRoom* pRoom;

	pUser->Logging(en_EnterRoomRequest, accountNo, roomNum, pUser->_state);

	auto roomIt = _roomManager.find(roomNum);
	if (roomIt == _roomManager.end())	// 없어
	{
		flag = 0;
		*pResponsePacket << type << accountNo << flag << roomNum;
	}
	else
	{
		WORD type = en_EnterRoomResponse;
		pRoom = roomIt->second;


		pRoom->AddUser(accountNo, pUser);
		pUser->_inRoom = roomNum;
		pUser->ChangePositionSpectator();
		BYTE roomLen = pRoom->GetCurrentRoomName().size();
		if (pRoom->IsGameing())
			flag = 2;
		*pResponsePacket << type << accountNo << flag << roomNum << roomLen;
		pResponsePacket->PutData((char*)pRoom->GetCurrentRoomName().c_str(), roomLen);
	}
	SendMessages(sID, pResponsePacket);
	pResponsePacket->subRef();

	if (flag)
	{
		CPacket* pBroadPacket = CPacket::Alloc();
		pBroadPacket->AddRef();
		pBroadPacket->Clear();

		WORD type = en_EnterRoomStateResponse;
		BYTE nickLen;
		std::string nick = userIt->second->GetMyNickname();
		nickLen = nick.size();
		*pBroadPacket << type << nickLen;
		pBroadPacket->PutData((char*)nick.c_str(), nickLen);

		SendRoomAll(roomNum, pBroadPacket);

		/*for (auto userIt = pRoom->GetUserList().begin(); userIt != pRoom->GetUserList().end(); ++userIt)
		{
			SendMessages(userIt->second->_sessionId, pResponsePacket);
		}*/

		pBroadPacket->subRef();
	}
}

//void CServer::ConnectToMonitorServer()
//{
//	//monitorClientPtr->TryToConnectServer(L"127.0.0.1", 12345, 1, true);
//}

void CServer::LeaveRoomResponse(unsigned __int64 sID, CPacket* pRecvPacket)
{
	uintptr_t accountNo;
	// -----
	// len | type ||| flag roomid roomLen roomName

	*pRecvPacket >> accountNo;
	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
	{
		DebugBreak();
	}

	CPacket* pResponsePacket;
	
	pResponsePacket = pRecvPacket;
	pResponsePacket->Clear();
	BYTE flag = 1;
	WORD type = en_LeaveRoomResponse;
	CChatRoom* pRoom;

	CUser* pUser = userIt->second;
	WORD roomId = pUser->GetCurrentRoom();
	pUser->Logging(en_LeaveRoomRequest, accountNo, roomId, pUser->_state);
	auto roomIt = _roomManager.find(roomId);
	if (roomIt == _roomManager.end())	// 없어
	{
		DebugBreak();
	}
	else
	{
		WORD type = en_LeaveRoomResponse;
		pRoom = roomIt->second;

		
		pRoom->CancelPlayer(pUser->_state, accountNo, sID);
		pUser->ReadyClear();

		pUser->RemovePosition();
		pRoom->EraseUser(accountNo);
		//pRoom->AddUser(accountNo, userIt->second);
		BYTE roomLen = pRoom->GetCurrentRoomName().size();

		pUser->LeaveRoom();
		/*userIt->second->_inRoom = 0;*/
		


		*pResponsePacket << type << accountNo << flag << roomId << roomLen;
		pResponsePacket->PutData((char *)pRoom->GetCurrentRoomName().c_str(), roomLen);
		
		if (pRoom->GetUserCount() == 0)
		{
			_roomManager.erase(roomId);
			roomPool.Free(pRoom);
		}
	}
	SendMessages(sID, pResponsePacket);
	pResponsePacket->subRef();

	if (flag)
	{
		CPacket* pBroadPacket = CPacket::Alloc();
		pBroadPacket->AddRef();
		pBroadPacket->Clear();

		WORD type = en_LeaveRoomStateResponse;
		BYTE nickLen;
		std::string nick = pUser->GetMyNickname();
		nickLen = nick.size();
		*pBroadPacket << type << nickLen;
		pBroadPacket->PutData((char*)nick.c_str(), nickLen);

		SendRoomAll(roomId, pBroadPacket);


		pBroadPacket->subRef();
	}
}

void CServer::ChatRoom(unsigned __int64 sID, CPacket* pPacket)
{
	uintptr_t accountNo;
	BYTE charLen;
	WORD roomId;
	char chat[128];
	// -----
	// len | type ||| flag roomid roomLen roomName

	*pPacket >> accountNo >> roomId >>charLen;
	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
		DebugBreak();

	CUser* pUser = userIt->second;
	if (accountNo != pUser->GetCurrentAccountNo())
		DebugBreak();

	pPacket->GetData(chat, charLen);
	chat[charLen] = 0;
	std::string chatString(chat);

	pPacket->Clear();
	BYTE flag = 1;
	WORD type = en_ChatResponse;
	CChatRoom* pRoom;

	if (roomId != pUser->GetCurrentRoom())
		DebugBreak();

	auto roomIt = _roomManager.find(roomId);
	if (roomIt == _roomManager.end())	// 없어
	{
		flag = 0;
	}
	else
	{
		pRoom = roomIt->second;
		BYTE roomLen = pRoom->GetCurrentRoomName().size();

	}
	
	pUser->Logging(en_CancelReadyRequest, accountNo, roomId, pUser->_state);

	if (flag)
	{
		CPacket* pBroadPacket = CPacket::Alloc();
		pBroadPacket->AddRef();
		pBroadPacket->Clear();

		BYTE nickLen;
		std::string nick = pUser->GetMyNickname();
		nickLen = nick.size();
		*pBroadPacket << type << accountNo << roomId << nickLen;
		pBroadPacket->PutData((char*)nick.c_str(), nickLen);

		BYTE chatLen = chatString.size();
		*pBroadPacket << chatLen;
		pBroadPacket->PutData((char*)chatString.c_str(), chatLen);

		SendRoomAll(roomId, pBroadPacket);


		pBroadPacket->subRef();
	}
	else
	{
		DebugBreak();
	}
	pPacket->subRef();
}

void CServer::ChangePosition(unsigned __int64 sID, CPacket* pPacket)
{
	// len(5) | type(2) roomNo(2) from(1) to(1) nicoLen(1) nickName(nickLen max 19)
	uintptr_t accountNo;
	USHORT roomNo;
	BYTE from, to;
	BYTE nickLen;

	//BYTE position;
	// -----
	
	// len = sizeof(type) + sizeof(accountNo) + sizeof(roomNo) + sizeof(from) + sizeof(to) + sizeof(nickLen) + nickLen;

	*pPacket >> accountNo;
	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
	{
		DebugBreak();
	}
	
	CUser* pUser = userIt->second;
	if (accountNo != pUser->GetCurrentAccountNo())
		DebugBreak();


	*pPacket >> roomNo;
	if (roomNo != pUser->_inRoom)
		DebugBreak();
	auto roomIt = _roomManager.find(roomNo);
	if (roomIt == _roomManager.end())	// 없어
	{
		DebugBreak();
	}
	
	pUser->Logging(en_Position, accountNo, roomNo, pUser->_state);
	CChatRoom* pRoom = roomIt->second;

	int a = 0;
	BYTE flag = true;
	// 가능할 수도... 
	if (pRoom->IsGameing())
	{
		flag = false;
		a = 1;
		// DebugBreak();	// 가능할 수 있음.
	}
	
	char buf[20];
	*pPacket >> from >> to;
	// TODO: 확인 필요할 듯
	if (pUser->GetCurrentState() == to)
	{
		DebugBreak();
	}
	
	if (to != 3)
	{
		// 추가할 것. 1이나 2에 사람이 있으면 안됨.
		if (!pRoom->IsPossibleChangePositionPlayer(to))
		{
			flag = false;
			a = 2;
		}
		*pPacket >> nickLen;
		pPacket->GetData(buf, nickLen);
		buf[nickLen] = 0;
		std::string nickName(buf);
	}
	


	if (flag)
	{
		switch (to)
		{
		case 1:
			pUser->ChangePositionPlayer1();

			break;
		case 2:
			pUser->ChangePositionPlayer2();
			break;
		case 3:
			pUser->ChangePositionSpectator();
			break;
		default:
			DebugBreak();
			break;
		}
	}
	
	// client에 보낼 packet 초기화
	pPacket->Clear();
	
	WORD type;


	// 통과
	if (to == 3)
	{
		type = en_ChangePositionSpectatorResponse;
		*pPacket << type << pUser->_accountNo << roomNo << flag << from << to;
		if (flag)
		{
			pRoom->CancelPlayer(from, accountNo, sID);
			pUser->ReadyClear();
			//pRoom->Player1Clear();

			SendRoomAll(roomNo, pPacket);
		}
		else
		{
			SendMessages(sID, pPacket);
		}
	}
	else if (to == 1 || to == 2)
	{
		type = en_ChangePositionPlayerResponse;
		*pPacket << type << pUser->_accountNo << roomNo << flag << from << to;
		
		if (flag)
		{
			pRoom->CancelPlayer(from, accountNo, sID);
			pRoom->ReadyPlayer(to, accountNo, sID);
			pUser->ReadyClear();


			std::string nick = pUser->GetMyNickname();
			nickLen = nick.size();
			*pPacket << nickLen;
			pPacket->PutData((char*)nick.c_str(), nickLen);
			SendRoomAll(roomNo, pPacket);
		}
		else
		{
			SendMessages(sID, pPacket);
		}
	}

	pPacket->subRef();
}

void CServer::Ready(unsigned __int64 sID, CPacket* pPacket)
{
	uintptr_t accountNo;
	WORD roomNo;
	BYTE position;
	// -----
	// 
	// input: type(2) accountNo(8) << roomNo(2) << position(1) << x(1) << y(1);
	// 
	// len | type ||| flag roomid roomLen roomName

	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
		DebugBreak();
	
	*pPacket >> accountNo;

	CUser* pUser = userIt->second;
	if (pUser->_accountNo != accountNo)
		DebugBreak();

	*pPacket >> roomNo;
	WORD roomId = pUser->GetCurrentRoom();
	if (roomNo != roomId)
		DebugBreak();


	auto roomIt = _roomManager.find(roomId);
	if (roomIt == _roomManager.end())	// 없어
		DebugBreak();
	CChatRoom* pRoom = roomIt->second;

	*pPacket >> position;
	pUser->Logging(en_ReadyRequest, accountNo, roomId, position);
	BYTE curPosition = pUser->GetCurrentState();
	if (position != curPosition)
		DebugBreak();

	if (curPosition != 1 && curPosition != 2)	// 1 2만 가능
	{
		DebugBreak();
	}

	if (pRoom->IsGameing())
	{
		DebugBreak();
	}


	
	// 전송 패킷 만들기.
	pPacket->Clear();

	WORD type = en_ReadyResponse;


	/*CPacket* pBroadPacket = CPacket::Alloc();
	pBroadPacket->AddRef();
	pBroadPacket->Clear();*/

	BYTE nickLen;
	std::string nick = pUser->GetMyNickname();
	nickLen = nick.size();
	BYTE flag = true;
	
	if (position == 1 || position == 2)
	{
		/*pRoom->ReadyPlayer(position, pUser->GetCurrentAccountNo(), sID);*/
		pUser->Ready();

		*pPacket << type << accountNo << roomId << position << flag;
		SendMessages(sID, pPacket);
		//SendRoomAll(roomId, pPacket);
		

		if (position == 1)
		{
			uintptr_t oppSessionId = pRoom->GetPlayer2SessionId();
			auto oppUserIt = userManager.find(oppSessionId);
			if (oppUserIt == userManager.end())
			{
				;
			}
			else
			{
				CUser* pOppUser = oppUserIt->second;
				if (pRoom->GetPlayer2AccountNo() != pOppUser->GetCurrentAccountNo())
					DebugBreak();

				BYTE oppReadyFlag = pOppUser->GetCurrentReadyFlag();
				if (oppReadyFlag)
				{

					pRoom->GameStart();
					pRoom->InitTurn();
					pRoom->NextTurn();
					pRoom->BoardClear();

					CPacket* pStartPacket = CPacket::Alloc();
					pStartPacket->AddRef();
					pStartPacket->Clear();
					WORD type = en_StartResponse;
					BYTE nextPosition = 1;
					*pStartPacket << type << accountNo << roomId << flag << nextPosition;

					SendRoomAll(roomId, pStartPacket);

					pStartPacket->subRef();
				}
			}

		}
		else
		{
			uintptr_t oppSessionId = pRoom->GetPlayer1SessionId();
			auto oppUserIt = userManager.find(oppSessionId);
			if (oppUserIt == userManager.end())
			{
				;
			}
			else
			{
				CUser* pOppUser = oppUserIt->second;
				if (pRoom->GetPlayer1AccountNo() != pOppUser->GetCurrentAccountNo())
					DebugBreak();


				BYTE oppReadyFlag = pOppUser->GetCurrentReadyFlag();
				if (oppReadyFlag)
				{
					pRoom->GameStart();
					pRoom->InitTurn();
					pRoom->NextTurn();
					pRoom->BoardClear();

					CPacket* pStartPacket = CPacket::Alloc();
					pStartPacket->AddRef();
					pStartPacket->Clear();
					WORD type = en_StartResponse;
					BYTE nextPosition = 1;
					*pStartPacket << type << accountNo << roomId << flag << nextPosition;
					SendRoomAll(roomId, pStartPacket);

					pStartPacket->subRef();
				}
			}
		}
		
		
	}
	else
	{
		DebugBreak();
	}
	pPacket->subRef();
	
}

void CServer::CancelReady(unsigned __int64 sID, CPacket* pPacket)
{
	uintptr_t accountNo;
	WORD roomNo;
	BYTE position;
	// -----
	// len | type ||| flag roomid roomLen roomName

	

	*pPacket >> accountNo >> roomNo >> position;
	auto userIt = userManager.find(sID);
	if (userIt == userManager.end())
		DebugBreak();
	

	CUser* pUser = userIt->second;
	if (accountNo != pUser->GetCurrentAccountNo())
		DebugBreak();


	WORD roomId = pUser->GetCurrentRoom();
	if (roomNo != roomId)
		DebugBreak();
	pUser->Logging(en_CancelReadyRequest, accountNo, roomId, position);

	auto roomIt = _roomManager.find(roomId);
	if (roomIt == _roomManager.end())	// 없어
	{
		DebugBreak();
	}

	CChatRoom* pRoom = roomIt->second;

	if (!(position == 1 || position == 2))
		DebugBreak();



	pPacket->Clear();
	
	// 통과
	WORD type = en_CancelReadyResponse;
	BYTE nickLen;
	std::string nick = pUser->GetMyNickname();
	nickLen = nick.size();
	BYTE flag = true;

	// 애매해질수도 시작했는데 줄수 있잖아
	if (pRoom->IsGameing())
	{
		flag = false;
	}
	
	if (position == 1 || position == 2)
	{
		//pRoom->CancelPlayer(position, pUser->GetCurrentAccountNo(), sID);
		pUser->CancelReady();

		*pPacket << type << accountNo << roomId << position;
		*pPacket << flag;
		SendMessages(sID, pPacket);
		

	}
	else
	{
		DebugBreak();
	}

	pPacket->subRef();

}

//void CServer::ConnectToMonitorServer()
//{
//	//monitorClientPtr->TryToConnectServer(L"127.0.0.1", 12345, 1, true);
//}


void CServer::SendRoomAll(int roomNum, CPacket* packet)
{
	// 주위 Sector 돌면서 SendMessage
	auto roomIt = _roomManager.find(roomNum);
	if (roomIt == _roomManager.end())
		return;
	
	CChatRoom* pRoom = roomIt->second;
	auto userList = pRoom->GetUserList();
	
	for (auto it = userList.begin() ; it != userList.end() ; ++it)
	{
		CUser *pUser = it->second;
		SendMessages(pUser->_sessionId, packet);
	}
}

// Ready, Start

// PutStone

void CServer::PutStone(unsigned __int64 sId, CPacket* pRecvPacket)
{
	uintptr_t accountNo;
	WORD roomNo;
	BYTE x, y;
	BYTE position;

	// type << accountNo << roomNo << position << x << y;

	*pRecvPacket >> accountNo >> roomNo >> position >> x >> y;
	auto userIt = userManager.find(sId);
	if (userIt == userManager.end())
		DebugBreak();

	CUser* pUser = userIt->second;
	if (accountNo != pUser->GetCurrentAccountNo())
	{
		DebugBreak();
	}

	WORD roomId = pUser->GetCurrentRoom();
	if (roomId != roomNo)
	{
		DebugBreak();
	}

	auto roomIt = _roomManager.find(roomNo);
	if (roomIt == _roomManager.end())
	{
		DebugBreak();
	}
	CChatRoom* pRoom = roomIt->second;


	BYTE flag = true;
	if (!pRoom->IsGameing())
	{
		flag = false;
	}

	// turn 확인 필요.
	BYTE curTurn = pRoom->GetCurrentTurn();
	if (curTurn != position)				
	{
		flag = false;
	}

	if (!(position == 1 || position == 2))
	{
		flag = false;
	}

	// 좌표 계산 + 이미 두었는지 확인.
	if (!pRoom->PossibleStone(x, y))
	{
		flag = false;
	}

	pRecvPacket->Clear();
	WORD type = en_PutStoneResponse;
	*pRecvPacket << type << accountNo << roomId << flag;

	if (flag)
	{

		pRoom->PutStone(x, y, position);
		pRoom->ProcessRecord(x, y);
		pRoom->NextTurn();

		*pRecvPacket << x << y << position;
		if (position == 1)
		{
			BYTE nextPosition = 2;
			*pRecvPacket << nextPosition;
		}
		else if (position == 2)
		{
			BYTE nextPosition = 1;
			*pRecvPacket << nextPosition;
		}
		else
		{
			DebugBreak();
		}
		SendRoomAll(roomId, pRecvPacket);

		// 이겼는지 판단.

		bool gameEndFlag = pRoom->GameEnd(x, y, position);
		if (gameEndFlag)
		{
			CUser* pOppUser = nullptr;
			if (position == 1)
			{
				uintptr_t oppSessionId = pRoom->GetPlayer2SessionId();
				auto oppUserIt = userManager.find(oppSessionId);
				if (oppUserIt == userManager.end())
				{
					DebugBreak();
				}

				pUser->Win();
				pOppUser = oppUserIt->second;
				if (pRoom->GetPlayer2AccountNo() != pOppUser->GetCurrentAccountNo())
					DebugBreak();
				pOppUser->Lost();
				pOppUser->ReadyClear();
			}
			else if (position == 2)
			{
				uintptr_t oppSessionId = pRoom->GetPlayer1SessionId();
				auto oppUserIt = userManager.find(oppSessionId);
				if (oppUserIt == userManager.end())
				{
					DebugBreak();
				}

				pUser->Win();
				pOppUser = oppUserIt->second;
				if (pRoom->GetPlayer1AccountNo() != pOppUser->GetCurrentAccountNo())
					DebugBreak();
				pOppUser->Lost(); 
				pOppUser->ReadyClear();
			}
			if (position == 3)
				DebugBreak();
			pUser->ReadyClear();
			
			pRoom->GameEnd();

			// 패킷 쏘기
			CPacket* pGamePacket = CPacket::Alloc();
			pGamePacket->AddRef();
			pGamePacket->Clear();
			WORD type = en_GameOverResponse;
			BYTE nickLen;

			*pGamePacket << type << roomId << position;

			const std::string winNick = pUser->GetMyNickname();
			nickLen = winNick.size();
			*pGamePacket << nickLen;
			pGamePacket->PutData((char *)winNick.c_str(), nickLen);

			const std::string loseNick = pOppUser->GetMyNickname();
			nickLen = loseNick.size();
			*pGamePacket << nickLen;
			pGamePacket->PutData((char*)loseNick.c_str(), nickLen);

			SendRoomAll(roomId, pGamePacket);
			pGamePacket->subRef();
		}
	}
	else
	{
		SendMessages(sId, pRecvPacket);
	}
	pRecvPacket->subRef();
	// [좌표 계산]
	// 1. 좌표가 밖인지 조사
	// 2. 이미 돌이 있는지 조사

	// 4. 돌 두기.
	// 스택에 저장해야 함.(player 가 넣었는지 저장).
	// 4-1. 응답 주기.
	// 4-2. 끝났는지 확인.
	// 4-3. 끝났다고 알려주기.
	// 4-4. DB 저장 요청.

	// 
	
}


//void ItemShop()	// ItemList 주기.
//{
//	// Item List를 줘야함.
//	// 번호, 아이템 이름, 가격, 설명.
//}
//
//void purchaseItem(uintptr_t sId, CPacket *pPacket)	// 아이템 구매 패킷 받음.
//{
//
//	// accountNo랑 itemId랑, 가격, 수량
//
//	// 있는 애인지
//
//
//	// 아이템id를 통해서 가격이 얼마인지 확인 잘못된 패킷 확인
//
//	// 돈이 있는지 검사.
//
//
//	// 차감하고, DB에 저장.
//
//	// accountNo, itemId, 수량, 얼마 차감. 로그 기록
//	// accountNo 가 획득 아이템 수량에 넣어줌.
//
//	// Response 넣기.
//}









// Item 상점
// 1. 리스트 가격 주기
// 2. 
// 
// Item 쓰기는 게임이 끝나고 차감.
// 



// 

// ItemShop에 관한 클래스가 하나 나와야 함.
// DB에 관한 클래스가 나와야함.
// 모니터링에 관한 클래스가 나와야 함.


// Todo 7-29
// PutStone 마무리


// item List
//itemid, item이름, 설명, 골드
//1. 무르기 권
//2. 시간 연장권
//3. 닉네임 변경권
//4. 초상화
//
//아이템_유저
//유저    아이템ID  갯수
//hena     1         -> delete
//
//
//
//
//아이템로그
//유저	아이템id 시간 구매/사용
//hena	1         1	  1
//hena	2		  2   2
//hena	1         2

