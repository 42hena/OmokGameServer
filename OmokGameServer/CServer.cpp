#include <iostream>
#include <process.h>

#include <unordered_map>
#include <list>
#include <string>
#include <set>

#include <algorithm>
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

#include "CNetLibrary.h"
#include "CServer.h"

#include "Protocol.h"
#include "CPDH.h"

CLFQueue<SJob> jobQ(500'001);

std::unordered_map<unsigned __int64, CUser*> userManager;
//std::unordered_map<uintptr_t, uintptr_t> accountKey;

alignas (64) uintptr_t messageRecvCount = 0;
alignas (64) uintptr_t messageSendCount = 0;
alignas (64) uintptr_t loginPacket = 0;
alignas (64) uintptr_t messagePacket = 0;
alignas (64) uintptr_t sectorPacket = 0;

// CServer
CServer::CServer()
	: userCount(0), updateCount(0),

	_createRoomPacketCount(0),
	_enterRoomPacketCount(0),
	_leaveRoomPacketCount(0),
	_chatRoomPacketCount(0)

{
	int errCode;
	int idx;
	// -----

	// Test 용도
	for (int i = 1; i <= 500; ++i)
	{
		auto pRoom = _roomPool.Alloc();
		pRoom->ChangeRoomNumber(i);
		_roomManager.insert({ i, pRoom });
	}


	wprintf(L"CServer Constructor\n");
	hUpdateTh = (HANDLE)_beginthreadex(nullptr, 0, UpdateThread, this, 0, nullptr);
	if (hUpdateTh == nullptr)
	{
		errCode = GetLastError();
		wprintf(L"hUpdateTh Fail %d\n", errCode);
		return;
	}
	hMonitorTh = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
	if (hMonitorTh == nullptr)
	{
		errCode = GetLastError();
		wprintf(L"hMonitorTh Fail %d\n", errCode);
		return;
	}
}

CServer::~CServer()
{
	WaitForSingleObject(hUpdateTh, INFINITE);
	WaitForSingleObject(hMonitorTh, INFINITE);
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

CChatRoom* CServer::FindRoom(WORD roomNo)
{
	auto iter = _roomManager.find(roomNo);
	if (iter == _roomManager.end())
	{
		DebugBreak();
		return (nullptr);	// 존재 안함
	}
	return iter->second;
}

CChatRoom* CServer::FindRoomTmp(WORD roomNo)
{
	auto iter = _roomManager.find(roomNo);
	if (iter == _roomManager.end())
	{
		return (nullptr);	// 존재 안함
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
	uintptr_t accountNo = pUser->GetCurrentAccountNo();

	WORD roomNo = pUser->GetCurrentRoom();
	auto roomIt = _roomManager.find(roomNo);
	if (roomIt == _roomManager.end())
	{
		int a = 0;;
	}
	else
	{
		CChatRoom* pRoom = roomIt->second;
		pRoom->EraseUser(accountNo);
		RoomDown();

	}
	if (accountNo == 0)
		DebugBreak();
	int ss = _accountKey.erase(accountNo);
	if (ss == 0)
		DebugBreak();

	InterlockedDecrement(&userCount);
	userPool.Free(pUser);
}

unsigned int CServer::UpdateThread(LPVOID param)
{
	CServer* ptr = reinterpret_cast<CServer*>(param);
	DWORD WaitForSingleObjectRet;
	SJob job;
	int jobQRet;
	// =====

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


unsigned int CServer::MonitorThread(LPVOID param)
{
	CServer* ptr = reinterpret_cast<CServer*>(param);
// -----

	static DWORD firstTime;
	DWORD lastTime;
	DWORD sec, min, hour;

	long flag, cpu, mem, sessionCnt, playerCnt, updateTPS, packetPoolCnt, JobPoolCnt;
	int total;
	char filename[256];
	// start
	firstTime = timeGetTime();
	
	FILE* fp;
	time_t currentTime;
	struct tm localTime;

	
	CProcessPDH processPdh(L"OmokGameServer");


	for ( ; ; )
	{
		currentTime = time(NULL);
		localtime_s(&localTime, &currentTime);
		
		sprintf_s(filename, sizeof(filename), "file_%04d-%02d-%02d_%02d.txt",
			localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
			localTime.tm_hour);
		/*fopen_s(&fp, filename, "a+");
		if (fp == nullptr)
		{
			DebugBreak();
			return 1;
		}*/
		lastTime = timeGetTime();
		total = (lastTime - firstTime) / 1000;
		sec = total % 60;
		total /= 60;
		min = total % 60;
		total /= 60;
		hour = total % 60;
		
		total = 0;
		
		flag = 0;
		// CPU Mem 줄 cpu, mem
		

		//flag, cpu, mem, sessionCnt, playerCnt, updateTPS, packetPoolCnt, JobPoolCnt;
		//sessionCnt = ptr->GetUser();
		//playerCnt = accountManager.size();// 삭제

		//updateTPS = ptr->GetMonitoringUpdateCount();
		//packetPoolCnt = CPacket::GetUseNode();
		//JobPoolCnt = jobQ.GetSize();

		

		wprintf(L"================================================================================\n");
		wprintf(L"S: Stop Or Play(Not implemented) | Q: Quit\n");
		wprintf(L"================================================================================\n");
		wprintf(L"Running Time:%3d h - %2d m - %2d s | server Status %d\n", hour, min, sec, flag);
		wprintf(L"================================================================================\n");
		wprintf(L"Session   : %d\n", ptr->GetSessionCount());
		wprintf(L"User      : %d\n", static_cast<int>(ptr->GetUserCount()));
		wprintf(L"In Lobby  : %d\n", ptr->GetLobbyUserCount());
		wprintf(L"In Room   : %d\n", ptr->GetRoomUserCount());
		wprintf(L"================================================================================\n");
		wprintf(L"Accept TPS: %d\n", ptr->GetAndInitAcceptTPS());
		wprintf(L"Recv   TPS: %d\n", ptr->GetAndInitRecvMessageTPS());
		wprintf(L"Send   TPS: %d\n", ptr->GetAndInitSendMessageTPS());
		wprintf(L"Update TPS: %d\n", 0);// 추가
		wprintf(L"================================================================================\n");

		//wprintf(L"Thread count: %d\n", processPdh.GetThreadCount());
		wprintf(L"Packet Type\n");
		wprintf(L"| login  : %5d | create:%5d enter : %5d | leave:%5d chat: %5d |\n", 0, ptr->GetAndInitCreateRoomPacketCount(), ptr->GetAndInitEnterRoomPacketCount(), ptr->GetAndInitLeaveRoomPacketCount(), ptr->GetAndInitChatRoomPacketCount());
		wprintf(L"| change : %5d | ready :%5d Cancel: %5d | PlaceStone      : %5d |\n", 0,0,0,0);
		wprintf(L"================================================================================\n");
		wprintf(L"CPacket GetTotalBucket:%d GetUseBucket:%d  GetTotalNode:%d GetUseNode:%d\n", CPacket::GetTotalBucket(), CPacket::GetUseBucket(), CPacket::GetTotalNode(), CPacket::GetUseNode());
		wprintf(L"JobQ Total:%d Pool:%d\n", jobQ.GetPoolAllocSize(), jobQ.GetSize());
		wprintf(L"================================================================================\n");
		// Thread, CPU, CPU, Memory
		wprintf(L"Thread count: %d\n", processPdh.GetThreadCount());
		wprintf(L"Whole CPU   : %d\n", processPdh.GetTotalCpuInteger());
		wprintf(L"Use   CPU   : %d\n", processPdh.GetUserCpuInteger());
		wprintf(L"NP          : %d\n", processPdh.GetNonPaged());
		wprintf(L"Memory      : %d\n", processPdh.GetPrivateMem());
		wprintf(L"================================================================================\n\n");
		Sleep(999);
	}

	return (0);
}

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

void CServer::MakeResponseLoginPacket(CUser* pUser, CPacket* pPacket)
{
	const auto type = static_cast<WORD>(en_PACKET_CS_CHAT_RES_LOGIN);	// type: int -> (WORD)
	BYTE status = 0;
	// =====

	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}
	auto accountNo = pUser->GetCurrentAccountNo();				// type: (uintptr_t)
	std::wstring nick = pUser->GetMyNickname();
	auto nickLen = static_cast<BYTE>(nick.size());	// type: size_t -> BYTE
	status = 1;

	// Init packet
	pPacket->Clear();
	
	// Set Packet
	*pPacket << type << accountNo << nickLen;
	pPacket->PutData(reinterpret_cast<const char*>(nick.c_str()), nickLen * 2);
	*pPacket << status;
}

void CServer::LoginProcedure(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	BYTE recvNickLen;
	WCHAR nickNameBuffer[20];	// 19자가 최대임.
	// -----

	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	// packet 값 빼기.
	*pPacket >> recvAccountNo >> recvNickLen;
	pPacket->GetData(reinterpret_cast<char *>(nickNameBuffer), recvNickLen * 2);	// const char*면 안되는 이유 찾아.
	nickNameBuffer[recvNickLen] = 0;


	// 이미 접속.
	auto it = _accountKey.find(recvAccountNo);
	if (it != _accountKey.end())
	{
		SSession* s = FindSession(it->second);
		wprintf(L"IP:%s port:%d\n", s->clientIP, s->port);
		wprintf(L"-----------------------------------------------------\n");
		wprintf(L"-----------------------------------------------------\n");
		wprintf(L"-----------------------------------------------------\n");
		DebugBreak();
		
		//1 방금 접속한 사람 끊기
		Disconnect(id);
		return;
	}

	if (recvNickLen > 19)	// enum으로 빼기.
	{
		DebugBreak();
		return;
	}

	// user member variable 초기화
	pUser->_accountNo = recvAccountNo;
	pUser->_nickName = nickNameBuffer;

	// 중복 체크용
	_accountKey.insert({ recvAccountNo, id });

	LobbyUp();

	// | type(2) | accountNo(8) nickLen(1) nickName(20) status(1)
	MakeResponseLoginPacket(pUser, pPacket);

	// sendPacket 전송 요청
	SendMessages(id, pPacket);
	pPacket->subRef();
}

void CServer::Recv(unsigned __int64 id, CPacket* pPacket)
{
	WORD type;
	// =====

	// Error Check
	if (pPacket == nullptr)
	{
		DebugBreak();
		return;
	}


	*pPacket >> type;
	switch (type)
	{
	case en_LoginRequest:	// ***!!
	{
		LoginProcedure(id, pPacket);
		break;
	}
	//case en_HeartBeatRequest:
	//{
	//	break;
	//}
	//case en_RoomListRequest:	// 보류	
	//{
	//	break;
	//}
	case en_CreateRoomRequest:	// ***
	{
		CreateCountUp();
		CreateRoomProcedure(id, pPacket);
		break;
	}
	case en_EnterRoomRequest:	// ***
	{
		EnterCountUp();
		EnterRoomProcedure(id, pPacket);
		break;
	}
	case en_LeaveRoomRequest:	// ***
	{
		LeaveCountUp();
		LeaveRoomProcedure(id, pPacket);
		break;
	}
	case en_ChatRequest:	// ***
	{
		ChatCountUp();
		ChattingProcedure(id, pPacket);
		break;
	}
	case en_ChangePositionPlayerRequest:
		ChangePosition(id, pPacket);
		break;
	case en_ChangePositionSpectatorRequest:
		ChangePosition(id, pPacket);
		break;
	
	case en_ReadyRequest:
	{
		Ready(id, pPacket);
		break;
	}
	case en_CancelReadyRequest:
	{
		CancelReady(id, pPacket);
		break;
	}
	
	case en_PutStoneRequest:
	{
		PutStone(id, pPacket);
		break;
	}
	case 59000:
	{
		GracefulShutdown(id, pPacket);
		break;
	}

	default:
		DebugBreak();
		Disconnect(id); // Attack
		pPacket->subRef();
		break;
	}
}

void CServer::MakeResponseGetRoomList(USHORT idx, CUser* pUser, CPacket* pPacket)
{
	const auto packetType  = static_cast<WORD>(en_RoomListResponse);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	auto accountNo = pUser->GetCurrentAccountNo();
	BYTE count = 0;
	*pPacket << packetType << accountNo << idx;

	auto roomSize = _roomManager.size();
	if (roomSize > 0)
	{
		int maxIdx = (roomSize - 1) / 10 + 1;
		count = min(roomSize - 10 * (idx - 1), 10);
		auto it = _roomManager.begin();
		std::advance(it, count - 1);
		for (int i = 0; i < count && it != _roomManager.end(); ++i) {
			auto pRoom = it->second;
			auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
			++it;
		}
	}
	else
		*pPacket << count;
}

void CServer::RoomList(unsigned __int64 id, CPacket* pRecvPacket)
{
	uintptr_t recvAccountNo;
	USHORT recvIndex;
	// -----

	//// len | type | len, name[]
	//*pRecvPacket >> recvAccountNo >> recvIndex;


	//// CPacket 가공
	//CPacket* pResponsePacket;
	//const USHORT type = en_RoomListResponse;
	//pResponsePacket = pRecvPacket;


	//// len | type(2) | accountNo(8) roomCount(2), [num(2)]
	//pResponsePacket->Clear();
	//WORD roomCount = _roomManager.size();


	//int r = roomCount % 40;
	//int q = roomCount / 40;
	//if (index < q)
	//{
	//	roomCount = 40;
	//}
	//else if (index == q)
	//{
	//	roomCount = r;
	//}
	//else
	//	roomCount = 0;
	//*pResponsePacket << type << accountNo << roomCount;

	//int cnt = 0;
	//for (auto it = _roomManager.begin(); it != _roomManager.end(); ++it)
	//{
	//	// TODO
	//	if (cnt >= 40 * (index) && cnt < 40 * (index + 1))
	//	{
	//		CChatRoom* pRoom = it->second;
	//		WORD lll = pRoom->GetCurrentRoomNumber();
	//		*pResponsePacket << lll;
	//		BYTE ss = pRoom->GetCurrentRoomName().size();
	//		*pResponsePacket << ss;
	//		pResponsePacket->PutData((char*)pRoom->GetCurrentRoomName().c_str(), ss);
	//	}
	//	cnt++;
	//}
	//SendMessages(sID, pResponsePacket);

	//pResponsePacket->subRef();
}

// | type(2) | accountNo(8) roomNo(2) roomLen(1) roomName(20) status(1)
void CServer::MakeCreateRoomPacket(CUser* pIser, CPacket* pPacket, WORD roomNo)
{
	const auto type = static_cast<WORD>(en_CreateRoomResponse);	// type: int -> WORD
	// =====

	if (pIser == nullptr)
	{
		DebugBreak();
		return;
	}
	
	pPacket->Clear();

	auto accountNo = pIser->GetCurrentAccountNo();	// type: (uintptr_t)

	// Setting Packet
	*pPacket << type << accountNo << roomNo;
}

//CPacket* CServer::MakeCreateRoomPacket(CUser* pUser, WORD roomNo)
//{
//	const auto type = static_cast<WORD>(en_CreateRoomResponse);	// type(WORD)
//
//	if (pUser == nullptr)
//	{
//		DebugBreak();
//		return nullptr;
//	}
//
//	auto pCreateRoomPacket = InitPacket();			// type(CPacket *)
//	auto accountNo = pUser->GetCurrentAccountNo();	// type(uintptr_t)
//
//	*pCreateRoomPacket << type << accountNo << roomNo;
//
//	return pCreateRoomPacket;
//}


CPacket* CServer::MakeEnterRoomAlarmPacket(CUser* user, CChatRoom* room)
{
	return nullptr;
}

void CServer::MakeResponseGracefulShutdownPacket(CUser* pUser, CPacket* pPacket)
{
	const auto type = static_cast<WORD>(en_GracefulShutdownResponse);	// type(WORD)
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	auto accountNo = pUser->GetCurrentAccountNo();	

	*pPacket << type << accountNo;

}

void CServer::GracefulShutdown(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	// =====
	*pPacket >> recvAccountNo;

	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}
	if (pUser->_accountNo != recvAccountNo)
		DebugBreak();
	if (pUser->GetCurrentRoom() != 0)
		DebugBreak();
	if (pUser->GetCurrentState() != 0)
		DebugBreak();

	MakeResponseGracefulShutdownPacket(pUser, pPacket);
	
	SendResponseMessage(id, pPacket);
	//SendMessages(id, pPacket);
}

void CServer::CreateRoomProcedure(unsigned __int64 id, CPacket* pPacket)
{
	DebugBreak();
	static USHORT roomNum = 0;

	uintptr_t recvAccountNo;
	// =====

	*pPacket >> recvAccountNo;

	auto pRoom = _roomPool.Alloc();
	if (pRoom == nullptr)
	{
		DebugBreak();
		return ;
	}

	while (1)
	{
		++roomNum;
		if (roomNum == 0)
			continue;

		auto roomIt = _roomManager.find(roomNum);	// FindRoom 찾아보기.
		if (roomIt == _roomManager.end())
		{
			break;
		}
	}
		
	pRoom->ChangeRoomNumber(roomNum);

	// 검증
	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}
	
	if (recvAccountNo != pUser->GetCurrentAccountNo())
		DebugBreak();

	// 묶고 싶은데...
	_roomManager.insert({ roomNum , pRoom });
	pRoom->ChangeRoomNumber(roomNum);
	pRoom->AddUser(recvAccountNo, pUser );
	//pUser->_inRoom = roomNum;
	pUser->EnterRoom(roomNum);
	pUser->ChangePositionSpectator();

	LobbyDown();
	RoomUp();

	MakeCreateRoomPacket(pUser, pPacket, roomNum);

	//auto pPacket = MakeCreateRoomPacket(pUser, roomNum);	// type(CPacket *)
	SendResponseMessage(id, pPacket);
}

//CPacket* CServer::MakeEnterRoomPacket(CUser* pUser, CChatRoom* pRoom, BYTE status)
//{
//	const auto packetType = static_cast<WORD>(en_EnterRoomResponse);
//	if (pUser == nullptr || pRoom == nullptr)
//	{
//		DebugBreak();
//		return nullptr;
//	}
//
//	auto pEnterRoomPacket = InitPacket();
//	auto accountNo = pUser->GetCurrentAccountNo();	// type(uintptr_t)
//	auto roomNo = pRoom->GetCurrentRoomNumber();	// type(WORD)
//
//	if (status)
//	{
//		if (pRoom->IsGameing())
//			status = 2;
//	}
//	*pEnterRoomPacket << packetType << accountNo << roomNo << status;
//	return nullptr;
//}

void CServer::MakeResponseEnterRoomPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE status)
{
	const auto type = static_cast<WORD>(en_EnterRoomResponse);	// type: (WORD)
	USHORT roomNo = 0;
	// =====

	//if (pUser == nullptr || pRoom == nullptr)
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	auto accountNo = pUser->GetCurrentAccountNo();				// type: (uintptr_t)
	//auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());	// type: (int) -> USHORT
	if (status)
	{
		roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
		if (pRoom->IsGameing())
			status = 2;
	}

	// Setting Packet
	*pPacket << type << accountNo << roomNo << status;
}


void CServer::MakeRoomMembersPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket)
{
	const auto type = static_cast<WORD>(en_RoomMembers);
	// =====

	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	
	pPacket->Clear();

	auto accountNo = pUser->GetCurrentAccountNo();
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	auto numOfPeople = static_cast<BYTE>(pRoom->GetUserCount());
	// Setting Packet
	*pPacket << type << accountNo << roomNo << numOfPeople;

	for (auto userIt : pRoom->GetUserList())
	{
		auto pRoomUser = userIt.second;
		if (pUser != pRoomUser)	// 나 자신 빼야함.
		{
			std::wstring nick = pRoomUser->GetMyNickname();
			auto nickLen = static_cast<BYTE>(nick.size());	// type: size_t -> BYTE
			*pPacket << nickLen;
			pPacket->PutData(reinterpret_cast<const char*>(nick.c_str()), nickLen * 2);
		}
	}
}

// pUser 사용 안함.
void CServer::MakeEnterPlayerAlarmPacket(CChatRoom* pRoom, CPacket* pPacket)
{
	// 수정
	const auto type = static_cast<WORD>(305);

	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	auto p1 = pRoom->FindUser(pRoom->GetPlayer1AccountNo());
	auto p2 = pRoom->FindUser(pRoom->GetPlayer2AccountNo());
	BYTE cnt = pRoom->GetPlayerCount();
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	BYTE nickLen = 0;
	pPacket->Clear();

	*pPacket << type << roomNo << cnt;
	if (p1)
	{
		std::wstring nick = p1->GetMyNickname();
		nickLen = nick.size();
		*pPacket << (BYTE)1 << p1->GetCurrentReadyFlag() << nickLen;
		pPacket->PutData(reinterpret_cast<const char*>(nick.c_str()), nickLen * 2);
	}
	if (p2)
	{
		std::wstring nick = p2->GetMyNickname();
		nickLen = nick.size();
		*pPacket << (BYTE)2 << p2->GetCurrentReadyFlag() << nickLen;
		pPacket->PutData(reinterpret_cast<const char*>(nick.c_str()), nickLen * 2);
	}
	


}

// | type(2) | roomNo(2) nickLen(1) nickName(max 20)
void CServer::MakeEnterAlarmPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket)	// 303
{
	const auto type = static_cast<WORD>(en_EnterRoomStateResponse);
	// =====

	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	BYTE nickLen = 0;
	std::wstring nick = pUser->GetMyNickname();
	nickLen = nick.size();

	pPacket->Clear();

	*pPacket << type << roomNo << nickLen;
	pPacket->PutData(reinterpret_cast<const char *>(nick.c_str()), nickLen * 2);
}

// | type(2) | accountNo(8) roomNo(2) numOfPeople(2) | [nickLen(1) nickName(max 20)]
void CServer::MakeGetUserListPacket(CUser* user, CChatRoom* room, CPacket* packet)	// 304
{
	//if (user == nullptr || room == nullptr)
	//{
	//	DebugBreak();

	//	return;
	//}

	//WORD type = 304;
	//INT64 accountNo = user->GetCurrentAccountNo();
	//WORD roomNo = room->GetCurrentRoomNumber();
	//WORD numOfPeople;
	//BYTE nickLen;

	//packet->Clear();
	//numOfPeople = room->GetUserCount();
	//*packet << type << accountNo << roomNo << numOfPeople;


	//for (auto v : room->GetUserList())
	//{
	//	CUser* pUser = v.second;
	//	
	//	std::string nick = pUser->GetMyNickname();
	//	nickLen = nick.size();
	//	*packet << nickLen;
	//	packet->PutData((char*)nick.c_str(), nickLen);
	//}
}

// | type(2) | accountNo(8) roomNo(2) roomLen(1) roomName(max 20) status(1)
void CServer::MakeResponseLeaveRoomPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE status)
{
	const auto type = static_cast<WORD>(en_LeaveRoomResponse);	// type: (WORD)
	// =====

	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	auto accountNo = pUser->GetCurrentAccountNo();				// type: (uintptr_t)
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());	// type: (int) -> USHORT

	if (pRoom->IsGameing())
		status = 0;

	// Setting Packet
	*pPacket << type << accountNo << roomNo << status;
}
// | type(2) | roomNo(2) nickLen(1) nickName(max 20)
void CServer::MakeLeaveAlarmPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket)	// 403
{
	const auto packetType = static_cast<WORD>(en_EnterRoomStateResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	pPacket->Clear();


	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	const std::wstring nickName = pUser->GetMyNickname();
	auto nickLen = static_cast<BYTE>(nickName.size());
	
	*pPacket << packetType << roomNo << nickLen;
	pPacket->PutData(reinterpret_cast<const char*>(nickName.c_str()), nickLen * 2);
}

// | type(2) | accountNo(8) roomNo(2) nickLen(1) nickName(max 20) chatLen(1) chatName(max 255)
void CServer::MakeChatPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, const std::wstring& chat)
{
	const auto type = static_cast<WORD>(en_ChatResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	std::wstring nick = pUser->GetMyNickname();
	auto nickLen = static_cast<BYTE>(nick.size());
	auto accountNo = pUser->GetCurrentAccountNo();
	auto chatLen = static_cast<BYTE>(chat.size());

	// Set Packet
	*pPacket << type << accountNo << roomNo << nickLen;
	pPacket->PutData(reinterpret_cast<const char*>(nick.c_str()), nickLen * 2);
	*pPacket << chatLen;
	pPacket->PutData(reinterpret_cast<const char*>(chat.c_str()), chatLen * 2);
}



void CServer::SendResponseMessage(uintptr_t id, CPacket* pPacket)
{
	SendMessages(id, pPacket);
	pPacket->subRef();
}


void CServer::EnterSuccess(uintptr_t id, CUser* pUser, CChatRoom* pRoom)
{
	auto pUserListPacket = InitPacket();

	// | type(2) | accountNo(8) roomNo(2) numOfPeople(2) | [nickLen(1) nickName(max 20)]
	//MakeGetUserListPacket(pUser, pRoom, pUserListPacket);
	//MakeGetUserListPacket(pUser, pRoom);

	MakeRoomMembersPacket(pUser, pRoom, pUserListPacket);
	SendResponseMessage(id, pUserListPacket);
	
	pRoom->AddUser(pUser->GetCurrentAccountNo(), pUser);

	auto pAlarmPacket = InitPacket();

	// | type(2) | roomNo(2) nickLen(1) nickName(max 20)
	MakeEnterAlarmPacket(pUser, pRoom, pAlarmPacket);

	// SendRoomAll(roomNo, pAlarmPacket);
	SendRoomAll(pUser->GetCurrentRoom(), pAlarmPacket, pUser->_accountNo);

	pAlarmPacket->subRef();

	if (pRoom->GetPlayerCount() > 0)
	{
		auto pPlayerAlarmPacket = InitPacket();
		MakeEnterPlayerAlarmPacket(pRoom, pPlayerAlarmPacket);
		//MakeUserAlarm(pUser, pRoom, pPlayerAlarmPacket);
		SendResponseMessage(id, pPlayerAlarmPacket);
	}
}


void CServer::EnterRoomProcedure(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	WORD recvRoomNo;
	// =====

	*pPacket >> recvAccountNo >> recvRoomNo;

	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	auto status = static_cast<BYTE>(1);
	auto pRoom = FindRoomTmp(recvRoomNo);
	if (pRoom == nullptr)
	{
		status = 0;
	}
	else
	{
		if (recvRoomNo != pRoom->GetCurrentRoomNumber())
			DebugBreak();
		if (pUser->GetCurrentRoom())
			status = 0;
		if (pRoom->GetUserCount() >= 20)
			status = 0;
		if (status)
		{
			pUser->EnterRoom(recvRoomNo);
			pUser->ChangePositionSpectator();
		}
	}

	//auto pEnterRoomPacket = MakeEnterRoomPacket(pUser, pRoom, status);
	MakeResponseEnterRoomPacket(pUser, pRoom, pPacket, status);
	SendResponseMessage(id, pPacket);


	if (status)
	{
		LobbyDown();
		RoomUp();
		EnterSuccess(id, pUser, pRoom);
	}
}


void CServer::LeaveSuccess(uintptr_t id, CUser* pUser, CChatRoom* pRoom)
{
	const auto packetType = static_cast<WORD>(en_LeaveRoomStateResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	auto pBroadPacket = InitPacket();
	std::wstring nick = pUser->GetMyNickname();
	auto nickLen = static_cast<BYTE>(nick.size());
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	*pBroadPacket << packetType << roomNo << nickLen;
	pBroadPacket->PutData(reinterpret_cast<const char*>(nick.c_str()), nickLen * 2);

	SendRoomAll(roomNo, pBroadPacket, pUser->GetCurrentAccountNo());

	pBroadPacket->subRef();
}

void CServer::LeaveRoomProcedure(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	USHORT recvRoomNo;
	// =====

	*pPacket >> recvAccountNo >> recvRoomNo;
	
	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	if (recvAccountNo != pUser->GetCurrentAccountNo())
	{
		DebugBreak();
		return;
	}

	auto pRoom = FindRoom(recvRoomNo);
	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	if (recvRoomNo != pRoom->GetCurrentRoomNumber())
		DebugBreak();
	BYTE status = 1;
	if (pUser->GetCurrentRoom() == 0)
		status = 0;
		
	pRoom->CancelPlayer(pUser->GetCurrentState(), recvAccountNo, id);
	pUser->ReadyClear();

	pUser->RemovePosition();
	pRoom->EraseUser(recvAccountNo);

	pUser->LeaveRoom();
	

	MakeResponseLeaveRoomPacket(pUser, pRoom, pPacket, status);

	if (pRoom->GetUserCount() == 0)
	{
		//_roomManager.erase(recvRoomNo);
		//_roomPool.Free(pRoom);
	}
	
	SendResponseMessage(id, pPacket);

	if (status)
	{
		LobbyUp();
		RoomDown();
		LeaveSuccess(id, pUser, pRoom);
	}
}


void CServer::ChattingProcedure(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	BYTE recvCharLen;
	WORD recvRoomNo;
	WCHAR buf[128];
	CChatRoom* pRoom = nullptr;
	// -----
	// len | type ||| flag roomid roomLen roomName

	// | type(2) | accountNo(8) roomNo(2) chatLen(1) chatting(max 127)
	*pPacket >> recvAccountNo >> recvRoomNo >> recvCharLen;
	pPacket->GetData(reinterpret_cast<char *>(&buf), recvCharLen * 2);
	buf[recvCharLen] = 0;
	std::wstring chatting(buf);

	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	if (recvAccountNo != pUser->GetCurrentAccountNo())
	{
		DebugBreak();
		return;
	}

	if (recvRoomNo != pUser->GetCurrentRoom())
	{
		DebugBreak();
		return;
	}


	pRoom = FindRoom(recvRoomNo);
	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	MakeChatPacket(pUser, pRoom, pPacket, chatting);
	
	SendRoomAll(recvRoomNo, pPacket);

	pPacket->subRef();
}

void CServer::MakeResponseChangePositionPacket(CUser *pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE from, BYTE to, BYTE status)
{
	const auto type = static_cast<WORD>(en_ChangePositionPlayerResponse);
	// =====

	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	auto accountNo = pUser->GetCurrentAccountNo();
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	auto numOfPeople = static_cast<BYTE>(pRoom->GetUserCount());

	*pPacket << type << pUser->_accountNo << roomNo << from << to;
		
	const std::wstring nick = pUser->GetMyNickname();
	BYTE nickLen = nick.size();
		

	*pPacket << nickLen;
	pPacket->PutData((char*)nick.c_str(), nickLen*2);

	*pPacket << status;
}

void CServer::ChangeSuccess(uintptr_t id, CUser* pUser, CChatRoom* pRoom, BYTE to)
{
	pUser->ReadyClear();
	pRoom->CancelPlayer(pUser->GetCurrentState(), pUser->GetCurrentAccountNo(), id);

	switch (to)
	{
	case 1:
		pUser->ChangePositionPlayer1();
		pRoom->pCountUp();
		break;
	case 2:
		pUser->ChangePositionPlayer2();
		pRoom->pCountUp();
		break;
	case 3:
		pUser->ChangePositionSpectator();
		pRoom->pCountDown();
		break;
	default:
		DebugBreak();
		break;
	}
	if (to != 3)
		pRoom->ReadyPlayer(to, pUser->GetCurrentAccountNo(), id);// Ready가 아닌 Setting임.
}

// procedure
void CServer::ChangePosition(unsigned __int64 id, CPacket* pPacket)
{
	// len(5) | type(2) roomNo(2) from(1) to(1) nicoLen(1) nickName(nickLen max 19)
	uintptr_t recvAccountNo;
	USHORT recvRoomNo;
	BYTE from, to;
	BYTE nickLen;
	WCHAR nick[20];
	//BYTE position;
	// -----
	
	// len = sizeof(type) + sizeof(accountNo) + sizeof(roomNo) + sizeof(from) + sizeof(to) + sizeof(nickLen) + nickLen;

	auto userIt = userManager.find(id);
	if (userIt == userManager.end())
	{
		DebugBreak();
	}
	*pPacket >> recvAccountNo >> recvRoomNo >> from >> to;
	
	auto pUser = userIt->second;
	if (recvAccountNo != pUser->GetCurrentAccountNo())
		DebugBreak();
	if (recvRoomNo != pUser->GetCurrentRoom())
		DebugBreak();

	auto pRoom = FindRoom(recvRoomNo);
	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	
	BYTE flag = true;
	// 가능할 수도... 
	if (pRoom->IsGameing())
	{
		flag = false;
		DebugBreak();	// 가능할 수 있음.
	}
	
	if (pUser->GetCurrentState() == to || pUser->GetCurrentState() != from)
	{
		DebugBreak();
	}

	if (to != 3)
	{
		if (!pRoom->IsPossibleChangePositionPlayer(to))
		{
			flag = false;
		}
	}
	
	*pPacket >> nickLen;
	pPacket->GetData(reinterpret_cast<char*>(&nick), nickLen * 2);
	nick[nickLen] = 0;


	// ======================================================
	// client에 보낼 packet 초기화

	MakeResponseChangePositionPacket(pUser, pRoom, pPacket, from, to, flag);
	
	// 통과
	if (to >= 1 && to <= 3)
	{
		if (flag)
		{
			if (from == 1 || from == 2)
			{
				pRoom->pCountDown();
			}

			ChangeSuccess(id, pUser, pRoom, to);
			SendRoomAll(recvRoomNo, pPacket);
		}
		else
		{
			SendMessages(id, pPacket);
		}
	}
	pPacket->subRef();
}

void CServer::MakeResponseReadyPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket)
{
	const auto type = static_cast<WORD>(en_ReadyResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	pPacket->Clear();
	
	BYTE nickLen;
	std::wstring nick = pUser->GetMyNickname();
	nickLen = nick.size();
	BYTE flag = true;
	BYTE position = pUser->GetCurrentState();
	*pPacket << type << pUser->GetCurrentAccountNo() << pRoom->GetCurrentRoomNumber() << position << flag;
}


// RoomNO만
void CServer::MakeGameStartPacket(CUser* pUser, CChatRoom* pRoom,CPacket* pPacket)
{
	const auto packetType = static_cast<WORD>(en_StartResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	
	pPacket->Clear();
	*pPacket << packetType << pRoom->GetCurrentRoomNumber();
}

void CServer::GameStartProcedure(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket)
{
	if (pUser->GetCurrentState() == 1)
	{
		uintptr_t oppAccountNo = pRoom->GetPlayer2AccountNo();
		auto oppUser = pRoom->FindUser(oppAccountNo);
		if (oppUser && oppUser->GetCurrentReadyFlag())
		{
			pRoom->InitGameSetting();

			CPacket* pStartPacket = CPacket::Alloc();
			pStartPacket->AddRef();
			MakeGameStartPacket(pUser, pRoom, pStartPacket);
			SendRoomAll(pRoom->GetCurrentRoomNumber(), pStartPacket);
			pStartPacket->subRef();
		}
	}
	else
	{
		uintptr_t oppAccountNo = pRoom->GetPlayer1AccountNo();
		auto oppUser = pRoom->FindUser(oppAccountNo);
		if (oppUser && oppUser->GetCurrentReadyFlag())
		{
			pRoom->InitGameSetting();

			CPacket* pStartPacket = CPacket::Alloc();
			pStartPacket->AddRef();
			MakeGameStartPacket(pUser, pRoom, pStartPacket);
			SendRoomAll(pRoom->GetCurrentRoomNumber(), pStartPacket);
			pStartPacket->subRef();
		}
	}
}
// user 1,2 
void CServer::MakeGameOverPacket(CUser* pUser, CUser* pOppUser, CChatRoom* pRoom, CPacket* pPacket, int flag)
{
	const auto type = static_cast<WORD>(en_GameOverResponse);
	// =====

	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	pPacket->Clear();

	BYTE _f = flag;
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	*pPacket << type << roomNo << _f;
	std::wstring nick = pUser->GetMyNickname();
	BYTE nickLen = nick.size();
	*pPacket << nickLen;
	pPacket->PutData((char*)nick.c_str(), nickLen * 2);

	nick = pOppUser->GetMyNickname();
	nickLen = nick.size();
	*pPacket << nickLen;
	pPacket->PutData((char*)nick.c_str(), nickLen * 2);
}

void CServer::MakeRecordPacket(CChatRoom* pRoom, CPacket* pPacket)
{
	const auto type = static_cast<WORD>(en_GameOverRecordResponse);
	COmokBoard &board = pRoom->GetBoard();

	pPacket->Clear();
	auto roomNo = static_cast<USHORT>(pRoom->GetCurrentRoomNumber());
	//board.
	BYTE recordCnt = board.GetStoneCount();
	*pPacket << roomNo << recordCnt;
	for (int i = 0; i < recordCnt; ++i)
	{
		BoardPos pos = board.GetPos(i);
		BYTE x, y;
		x = pos._x;
		y = pos._y;
		*pPacket << x << y;
	}
	// room / size [x, y] BYTE
}


void CServer::Ready(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	WORD recvRoomNo;
	BYTE recvPosition;
	// =====
	// len | type ||| flag roomid roomLen roomName status

	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}

	*pPacket >> recvAccountNo >> recvRoomNo >> recvPosition;
	if (pUser->_accountNo != recvAccountNo)
		DebugBreak();

	auto roomNo = pUser->GetCurrentRoom();
	if (recvRoomNo != roomNo)
		DebugBreak();

	auto pRoom = FindRoom(recvRoomNo);
	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	BYTE curPosition = pUser->GetCurrentState();
	if ((recvPosition != curPosition) && curPosition > 2)
		DebugBreak();

	if (pRoom->IsGameing())
	{
		DebugBreak();
	}
	//pRoom->ReadyPlayer()
	//pRoom->CancelPlayer(recvPosition, pUser->GetCurrentAccountNo(), id);
	pUser->Ready();

	MakeResponseReadyPacket(pUser, pRoom, pPacket);
	SendRoomAll(roomNo, pPacket);
	pPacket->subRef();

	// Game 시작 부분 (내부에서 pPacket 안씀)
	GameStartProcedure(pUser, pRoom, pPacket);
}

void CServer::MakeResponseCancelPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE status)
{
	const auto packetType = static_cast<WORD>(en_CancelReadyResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	
	pPacket->Clear();
	
	BYTE nickLen;
	std::wstring nick = pUser->GetMyNickname();
	nickLen = nick.size();

	BYTE  position = pUser->GetCurrentState();
	*pPacket << packetType << pUser->GetCurrentAccountNo() << pRoom->GetCurrentRoomNumber() << position << status;
}

void CServer::CancelReady(unsigned __int64 id, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	WORD recvRoomNo;
	BYTE recvPosition;

	*pPacket >> recvAccountNo >> recvRoomNo >> recvPosition;
	
	auto pUser = FindUser(id);
	if (pUser == nullptr)
	{
		DebugBreak();
		return;
	}
	
	if (recvAccountNo != pUser->GetCurrentAccountNo())
		DebugBreak();

	auto roomNo = pUser->GetCurrentRoom();
	if (recvRoomNo != roomNo)
		DebugBreak();

	auto pRoom = FindRoom(roomNo);
	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	if (!(recvPosition == 1 || recvPosition == 2))
		DebugBreak();

	bool flag = true;
	if (pRoom->IsGameing())
	{
		flag = false;
	}
	
	MakeResponseCancelPacket(pUser, pRoom, pPacket, flag);

	if (flag)
	{
		pUser->CancelReady();
		// pRoom->CancelPlayer(recvPosition, pUser->GetCurrentAccountNo(), id); 자리 옮길때.
		SendRoomAll(pRoom->GetCurrentRoomNumber(), pPacket);
	}
	else
	{
		SendMessages(id, pPacket);
	}
	pPacket->subRef();
}

void CServer::SendRoomAll(USHORT roomNum, CPacket* packet)
{
	auto roomIt = _roomManager.find(roomNum);
	if (roomIt == _roomManager.end())
		return;
	
	auto pRoom = roomIt->second;
	auto userList = pRoom->GetUserList();
	for (auto it = userList.begin() ; it != userList.end() ; ++it)
	{
		auto *pUser = it->second;
		SendMessages(pUser->_sessionId, packet);
	}
}

void CServer::SendRoomAll(USHORT roomNum, CPacket* pPacket, UINT64 accountNo)
{
	// 주위 Sector 돌면서 SendMessage
	auto roomIt = _roomManager.find(roomNum);
	if (roomIt == _roomManager.end())
		return;

	auto pRoom = roomIt->second;	// type: (CChatRoom *)
	auto userList = pRoom->GetUserList();	// type: (unordered_map<unsigned long long, CUser*>)

	for (auto it = userList.begin(); it != userList.end(); ++it)
	{
		auto pUser = it->second;
		if (pUser->GetCurrentAccountNo() != accountNo)
			SendMessages(pUser->_sessionId, pPacket);
	}
	//pPacket->subRef();
}

void CServer::SendResponseMessageAll(int roomNum, CPacket* pPacket, UINT64 accountNo)
{
	SendRoomAll(roomNum, pPacket, accountNo);
	pPacket->subRef();
}

// Ready, Start

// PutStone

void CServer::MakeResponsePutStonePacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE flag, BYTE x, BYTE y)
{
	const auto packetType = static_cast<WORD>(en_PutStoneResponse);
	if (pUser == nullptr || pRoom == nullptr)
	{
		DebugBreak();
		return;
	}
	pPacket->Clear();

	BYTE position = pUser->GetCurrentState();
	*pPacket << packetType << pUser->GetCurrentAccountNo() << pRoom->GetCurrentRoomNumber() << flag;
	*pPacket << x << y << position;
}


void CServer::GameOverProcedure(CUser*pUser, CChatRoom* pRoom, int endFlag)
{
	// game 끝났다 보내기
	auto pGameOverPacket = CPacket::Alloc();
	pGameOverPacket->AddRef();
	
	if (pUser->GetCurrentState() == 1)
	{
		uintptr_t oppAccountNo = pRoom->GetPlayer2AccountNo();
		auto oppUser = pRoom->FindUser(oppAccountNo);
		MakeGameOverPacket(pUser, oppUser, pRoom, pGameOverPacket, endFlag);
	}
	else
	{
		uintptr_t oppAccountNo = pRoom->GetPlayer1AccountNo();
		auto oppUser = pRoom->FindUser(oppAccountNo);
		
		MakeGameOverPacket(pUser, oppUser,  pRoom, pGameOverPacket, endFlag);
	}
	SendRoomAll(pRoom->GetCurrentRoomNumber(), pGameOverPacket);
	pGameOverPacket->subRef();
	
	// Game 기수 정보 보내기. TODO
}
void CServer::PutStone(unsigned __int64 sId, CPacket* pPacket)
{
	uintptr_t recvAccountNo;
	WORD recvRoomNo;
	BYTE recvX, recvY;
	BYTE recvPosition;

	//// type << accountNo << roomNo << position << x << y;
	*pPacket >> recvAccountNo >> recvRoomNo >> recvPosition >> recvX >> recvY;

	auto userIt = userManager.find(sId);
	if (userIt == userManager.end())
		DebugBreak();

	auto pUser = userIt->second;
	if (recvAccountNo != pUser->GetCurrentAccountNo())
	{
		DebugBreak();
		return;
	}

	auto roomNo = pUser->GetCurrentRoom();
	if (recvRoomNo != roomNo)
	{
		DebugBreak();
		return;
	}

	auto pRoom = FindRoom(roomNo);
	if (pRoom == nullptr)
	{
		DebugBreak();
		return;
	}

	BYTE flag = true;
	if (!pRoom->IsGameing())
	{
		flag = false;
	}

	// turn 확인 필요.
	BYTE curTurn = pRoom->GetCurrentTurn();
	if (curTurn != (recvPosition - 1))
	{
		flag = false;
	}

	if (!(recvPosition == 1 || recvPosition == 2))
	{
		flag = false;
	}

	// 좌표 계산 + 이미 두었는지 확인.
	if (!pRoom->PossibleStone(recvX, recvY))
	{
		flag = false;
	}

	MakeResponsePutStonePacket(pUser, pRoom, pPacket, flag, recvX, recvY);

	if (flag)
	{
		pRoom->PlaceStoneWrapper(recvX, recvY, recvPosition);
		pRoom->TurnEnd();
		SendRoomAll(roomNo, pPacket);
		
		auto endFlag = pRoom->CheckGameOverWrapper(recvX, recvY, recvPosition);
		if (endFlag)
		{
			GameOverProcedure(pUser, pRoom, endFlag);
		}
	}
	else
	{
		SendMessages(sId, pPacket);
	}


	pPacket->subRef();
}
