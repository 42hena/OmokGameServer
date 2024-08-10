
#include <process.h>
#include <unordered_map>
#include <map>

#include <Windows.h>


#include "CPacket.h"
#include "CRBuffer.h"
#include "CLFQueue.h"
#include "CLFStack.h"
#include "Session.h"

#include "Job.h"

#include "Player.h"
#include "CGroup.h"
#include "CGroupManager.h"
#include "CNetLibrary.h"
#include "CGameServerInterface.h"

#include "ProfileManager.h"


CGroup::CGroup()
	: hGroupThread(nullptr),
	groupId(1),
	fps(0)
{
	hJobEvent = CreateEvent(nullptr, false, 0, nullptr);
	if (hJobEvent == nullptr)
	{
		DebugBreak();
	}

	hExitEvent = CreateEvent(nullptr, true, 0, nullptr);
	if (hExitEvent == nullptr)
	{
		DebugBreak();
	}

	// TODO:0506
	for (int i = 14999; i >= 0; i--)
	{
		indexAllocator.push(i);
	}

	wprintf(L"In CGroup\n");
	hGroupThread = (HANDLE)_beginthreadex(nullptr, 0, GroupThread, this, 0, nullptr);
	wprintf(L"cons End\n");
}


//CGroup::CGroup(int groupId)
//	: hGroupThread(nullptr),
//	groupId(groupId)
//{
//
//	hJobEvent = CreateEvent(nullptr, false, 0, nullptr);
//	if (hJobEvent == nullptr)
//	{
//		DebugBreak();
//	}
//
//	hExitEvent = CreateEvent(nullptr, true, 0, nullptr);
//	if (hExitEvent == nullptr)
//	{
//		DebugBreak();
//	}
//
//	wprintf(L"In CGroup\n");
//	hGroupThread = (HANDLE)_beginthreadex(nullptr, 0, GroupThread, this, 0, nullptr);
//	wprintf(L"cons End\n");
//}

CGroup::CGroup(int groupId, int mode)
	: hGroupThread(nullptr),
	groupId(groupId),
	mode(mode)
{

	hJobEvent = CreateEvent(nullptr, false, 0, nullptr);
	if (hJobEvent == nullptr)
	{
		DebugBreak();
	}

	hExitEvent = CreateEvent(nullptr, true, 0, nullptr);
	if (hExitEvent == nullptr)
	{
		DebugBreak();
	}

	// TODO:0506
	for (int i = 14999; i >= 0; i--)
	{
		indexAllocator.push(i);
	}

	wprintf(L"In CGroup\n");
	hGroupThread = (HANDLE)_beginthreadex(nullptr, 0, GroupThread, this, 0, nullptr);
	wprintf(L"cons End\n");
}

CGroup::CGroup(int groupId, int mode, int fps)
	: hGroupThread(nullptr),
	groupId(groupId),
	mode(mode),
	fps(fps)
{

	hJobEvent = CreateEvent(nullptr, false, 0, nullptr);
	if (hJobEvent == nullptr)
	{
		DebugBreak();
	}

	hExitEvent = CreateEvent(nullptr, true, 0, nullptr);
	if (hExitEvent == nullptr)
	{
		DebugBreak();
	}

	// TODO:0506
	for (int i = 14999; i >= 0; i--)
	{
		indexAllocator.push(i);
	}

	wprintf(L"In CGroup\n");
	hGroupThread = (HANDLE)_beginthreadex(nullptr, 0, GroupThread, this, 0, nullptr);
	wprintf(L"cons End\n");
}

CGroup::~CGroup()
{
	wprintf(L"In ~CGroup\n");
	WaitForSingleObject(hGroupThread, INFINITE);
	wprintf(L"End\n");
}


//long CGroup::GetFrame()
//{
//	return InterlockedExchange(&frame, 0);
//}


void CGroup::JoinGroup(unsigned __int64 sessionId, SSession* sessionPtr)
{
	SJob job;
	// -----

	//wprintf(L"JoinGroup\n");

	// Init job
	job.type = en_JOB_TYPE::en_JOB_ON_ROOM_JOIN;
	job.sessionId = sessionId;
	job.ptr = sessionPtr;

	// send Queue and Wake Event
	jobQ.Enqueue(job);
	if (SetEvent(hJobEvent) == 0)
	{
		wprintf(L"JoinGroup Fail%d\n", GetLastError());
		DebugBreak();
	}
	
}

void CGroup::ExitGroup(unsigned __int64 sessionId)
{
	//wprintf(L"CGroup::ExitGroup\n");

	auto it = index.find(sessionId);
	if (it == index.end())
		DebugBreak();

	int arrayIdx = it->second;
	SSession* sessionPtr = sessionPtrArrays[arrayIdx].sessionPtr;
	index.erase(it);

	sessionPtrArrays[arrayIdx].sessionPtr = nullptr;
	sessionPtrArrays[arrayIdx].flag = false;
}

void CGroup::ExitGroup(unsigned __int64 sessionId, int mode)
{
	/////wprintf(L"CGroup::ExitGroup\n");

	auto it = index.find(sessionId);
	if (it == index.end())
		DebugBreak();

	int arrayIdx = it->second;
	SSession* sessionPtr = sessionPtrArrays[arrayIdx].sessionPtr;
	index.erase(it);



	sessionPtr->mode = mode;
	sessionPtr->groupId = 0;

	sessionPtrArrays[arrayIdx].sessionPtr = nullptr;
	sessionPtrArrays[arrayIdx].flag = false;

	OnClientGroupExit(sessionId);
}

void CGroup::ExitGroup(SSession* sessionPtr)
{
	wprintf(L"ExitGroup\n");



	for (int i = 0; i < 15000; ++i)
	{
		if (sessionPtrArrays[i].sessionPtr == sessionPtr)
		{
			sessionPtrArrays[i].flag = false;
			sessionPtrArrays[i].sessionPtr = nullptr;
		}
	}

	/*SJob job;

	jobQ.Enqueue(job);
	long ret = SetEvent(hJobEvent);*/
}


void CGroup::PushIndex(int i)
{
	indexAllocator.push(i);
}
long CGroup::indexSize()
{
	return index.size();
}

unsigned int CGroup::GroupThread(LPVOID param)
{
	int lastTime = 0;
	//wprintf(L"%p\n", &lastTime);

	CGroup* thisPtr = reinterpret_cast<CGroup*>(param);
	SSession* session;
	int idx;
	wprintf(L"GroupThread thisPtr:%p tid:%d\n", thisPtr, GetCurrentThreadId());
	// frame 만들어야 함.

	/*HANDLE handleArrays[2];

	handleArrays[0] = thisPtr->hExitEvent;
	handleArrays[1] = thisPtr->hJobEvent;*/

	int code;
	int sessionIdx;
	int time;
	int fps = thisPtr->fps;
	while (1)
	{
		lastTime = min(lastTime, fps);
		Sleep(fps - lastTime);
		lastTime = timeGetTime();

		// 바꾼 이유-> job이 지우기 전에 먼저 처리
		for (sessionIdx = 0; sessionIdx < 15000; ++sessionIdx)
		{
			int packetSize;
			if (thisPtr->sessionPtrArrays[sessionIdx].flag == true)
			{
				session = thisPtr->sessionPtrArrays[sessionIdx].sessionPtr;
				if (session->mode == thisPtr->mode)
				{
					if (session->testFlag)
					{
						thisPtr->ExitGroup(session->sessionID, -1);
						if (InterlockedDecrement(&session->IOCount) == 0)
							thisPtr->netLib->ReleaseSession(session->sessionID);
						thisPtr->PushIndex(sessionIdx);
					}
					else
					{
						CPacket* packetArrays[200];
						packetSize = session->packetQ.GetSize();
						packetSize = min(packetSize, 200);

						for (idx = 0; idx < packetSize; ++idx)
						{
							code = session->packetQ.Dequeue(packetArrays[idx]);
							if (code == -1)
								DebugBreak();
						}
						if (packetSize != 0)
							thisPtr->OnMessage(session->sessionID, packetArrays, packetSize);
					}
				}
			}
		}

		code = thisPtr->jobQ.GetSize();
		SJob job;

		// JobQ 개수만큼 빼기
		for (int idx = 0; idx < code; ++idx)
		{
			thisPtr->jobQ.Dequeue(job);
			if (job.type == en_JOB_TYPE::en_JOB_ON_ROOM_JOIN)
			{
				int allocIdx = thisPtr->indexAllocator.top();
				thisPtr->indexAllocator.pop();

				if (thisPtr->sessionPtrArrays[allocIdx].flag == false)
				{
					thisPtr->sessionPtrArrays[allocIdx].flag = true;
					thisPtr->sessionPtrArrays[allocIdx].sessionPtr = (SSession*)job.ptr;

					thisPtr->index.insert({ job.sessionId , allocIdx });
					thisPtr->sessionPtrArrays[allocIdx].sessionPtr->mode = thisPtr->mode; //
				}

				thisPtr->OnClientGroupJoin(job.sessionId);
			}
		}

		
		time = timeGetTime();
		if (time - lastTime < 0)
		{
			wprintf(L"%d %d %d\n", lastTime, time, time -lastTime);
			DebugBreak();
		}
		lastTime = time - lastTime;
		InterlockedIncrement(&thisPtr->fps);
	}


	//	code = WaitForMultipleObjects(2, handleArrays, false, 100 - lastTime);
	//	lastTime = timeGetTime();
	//	if (code == WAIT_OBJECT_0)
	//	{
	//		break;
	//	}
	//	else if (code == WAIT_OBJECT_0 + 1)
	//	{
	//		while (1)
	//		{
	//			code = thisPtr->jobQ.Dequeue(job);

	//			if (code == -1)
	//			{
	//				break;
	//			}
	//			if (job.type == en_JOB_TYPE::en_JOB_ON_ROOM_JOIN)
	//			{
	//				int i = 0;
	//				for (i = 0; i < 15000; ++i)
	//				{
	//					if (thisPtr->sessionPtrArrays[i].flag == false)
	//					{
	//						thisPtr->sessionPtrArrays[i].flag = true;
	//						thisPtr->sessionPtrArrays[i].sessionPtr = (SSession*)job.ptr;
	//						
	//						thisPtr->index.insert({ job.sessionId , i });
	//						thisPtr->sessionPtrArrays[i].sessionPtr->mode = thisPtr->mode; //
	//						thisPtr->sessionPtrArrays[i].sessionPtr->groupId = thisPtr->groupId;
	//						
	//						// thisPtr->index.insert({job.sessionId, i}); TODO: 미정
	//						break;
	//					}
	//				}
	//				if (i == 15000)
	//					DebugBreak();
	//				thisPtr->OnClientGroupJoin(job.sessionId);
	//			}
	//		}
	//		CProfiler WAIT1(L"WAIT_OBJ");
	//		for (int i = 0; i < 15000; ++i)
	//		{
	//			int packetSize;
	//			session = thisPtr->sessionPtrArrays[i].sessionPtr;
	//			if (thisPtr->sessionPtrArrays[i].flag == true)
	//			{
	//				if (session->mode == thisPtr->mode)
	//				{
	//					if (session->testFlag)
	//					{
	//						thisPtr->ExitGroup(session->sessionID, -1);
	//						long ioCount = InterlockedDecrement(&session->IOCount);
	//						if (ioCount == 0)
	//							thisPtr->netLib->ReleaseSession(session->sessionID);
	//					}
	//					else
	//					{
	//						CPacket* packetArrays[200];
	//						packetSize = session->packetQ.GetSize();
	//						packetSize = min(packetSize, 200);

	//						for (idx = 0; idx < packetSize; ++idx)
	//						{
	//							code = session->packetQ.Dequeue(packetArrays[idx]);
	//							if (code == -1)
	//								break;
	//						}
	//						if (packetSize != 0)
	//							thisPtr->OnMessage(session->sessionID, packetArrays, packetSize);
	//					}
	//					//while (1)
	//					//{
	//					//	CPacket* packet;
	//					//	/*ProfileBegin(L"Dequeue");*/
	//					//	code = session->packetQ.Dequeue(packet);
	//					//	//ProfileEnd(L"Dequeue");
	//					//	if (code == -1)
	//					//		break;
	//					//	thisPtr->OnMessage(session->sessionID, packet);
	//					//}
	//				}
	//			}
	//			else
	//			{

	//			}
	//			
	//		}

	//		//InterlockedIncrement(&thisPtr->frame);
	//	}


	//	else if (code == WAIT_TIMEOUT)
	//	{
	//		CProfiler WAIT2(L"WAIT_TIMEOUT");
	//		int packetSize;
	//		for (int i = 0; i < 15000; ++i)
	//		{
	//			if (thisPtr->sessionPtrArrays[i].flag == true)
	//			{
	//				session = thisPtr->sessionPtrArrays[i].sessionPtr;
	//				if (session->mode == thisPtr->mode)
	//				{
	//					if (session->testFlag)
	//					{
	//						thisPtr->ExitGroup(session->sessionID, -1);
	//						long ioCount = InterlockedDecrement(&session->IOCount);
	//						if (ioCount == 0)
	//							thisPtr->netLib->ReleaseSession(session->sessionID);
	//					}
	//					else
	//					{
	//						CPacket* packetArrays[200];
	//						packetSize = session->packetQ.GetSize();
	//						packetSize = min(packetSize, 200);

	//						for (idx = 0; idx < packetSize; ++idx)
	//						{
	//							code = session->packetQ.Dequeue(packetArrays[idx]);
	//							if (code == -1)
	//								break;
	//						}
	//						if (packetSize != 0)
	//							thisPtr->OnMessage(session->sessionID, packetArrays, packetSize);
	//					}
	//					/*SSession* session = thisPtr->sessionPtrArrays[i].sessionPtr;
	//					while (1)
	//					{
	//						CPacket* packet;
	//						code = session->packetQ.Dequeue(packet);
	//						if (code == -1)
	//							break;
	//						thisPtr->OnMessage(session->sessionID, packet);
	//					}*/
	//				}
	//				/*else if (thisPtr->sessionPtrArrays[i].sessionPtr->mode == thisPtr->connectMode)
	//				{
	//					SSession* session = thisPtr->sessionPtrArrays[i].sessionPtr;
	//					thisPtr->OnClientGroupExit(session->sessionID);
	//				}*/
	//			}
	//		}
	//	}
	//	else if (code == WAIT_FAILED)
	//	{
	//		wprintf(L"%d\n", GetLastError());
	//		DebugBreak();
	//	}
	//	time = timeGetTime();
	//	if (time - lastTime < 0)
	//	{
	//		wprintf(L"%d %d %d\n", lastTime, time, time -lastTime);
	//		DebugBreak();
	//	}
	//	lastTime = time - lastTime;
	//	InterlockedIncrement(&thisPtr->fps);
	//	//Sleep(20);
	//}

	DebugBreak();
	wprintf(L"Group Thread Exit:%d\n", GetCurrentThreadId());
	return (0);
}

long CGroup::GetFrame()
{
	return InterlockedExchange(&fps, 0);
}

int CGroup::FindIndex(unsigned __int64 sessionId)
{
	auto it = index.find(sessionId);
	if (it == index.end())
		DebugBreak();
	return it->second;
}

SSession* CGroup::FindSessionPtr(int idx)
{
	return sessionPtrArrays[idx].sessionPtr;
}
//CLobby::CLobby()
//	/*: CGroup(1),
//	playerCount(0)*/
//{
//	//netLib = reinterpret_cast<CGameServerInterface*>(ptr);
//}


CLobby::CLobby(void* ptr)
	: CGroup(1, 2, 100)
	//playerCount(0)
{
	netLib = reinterpret_cast<CNetLibrary*>(ptr);
	gameFace = reinterpret_cast<CGameServerInterface*>(ptr);
}


//void CLobby::ExitGroup(unsigned __int64 sessionId)
//{
//	wprintf(L"CLobby::ExitGroup\n");
//
//
//	/*int arrayIdx = FindIndex(sessionId);
//	SSession* sessionPtr = FindSessionPtr(arrayIdx);
//	index.erase(it);
//
//	sessionPtrArrays[arrayIdx].sessionPtr = nullptr;
//	sessionPtrArrays[arrayIdx].flag = false;*/
//}

//
void CLobby::OnClientGroupJoin(unsigned __int64 sessionId)
{
	SPlayer* newPlayer;	// TODO
	// -----

	gameFace->SRWSLock();
	auto it = gameFace->playersMap.find(sessionId);
	newPlayer = it->second;
	gameFace->SRWSUnLock();


	// accountKey.insert({ newPlayer->accountNo, sessionId }); //0일거임.
	groupPlayer.insert({ sessionId, newPlayer });


	

	/*newPlayer = netLib->FindPlayer(sessionId);
	if (newPlayer == nullptr)
		DebugBreak();

	groupPlayer.insert({ sessionId, newPlayer });
	InterlockedIncrement(&playerCount);*/
}


void CLobby::OnClientGroupExit(unsigned __int64 sessionId)
{
	SPlayer* player;

	auto it = groupPlayer.find(sessionId);
	if (it == groupPlayer.end())
		DebugBreak();

	player = it->second;
	size_t count = accountKey.erase(player->accountNo);
	if (player->accountNo  != -1 && count == 0)
	{
		DebugBreak();
	}

	count = groupPlayer.erase(sessionId);
	if (count == 0)
		DebugBreak();
}

//long CLobby::GetGroupPlayerCount()
//{
//	return playerCount;
//}
//
//long CLobby::GetGroupPlayerMapCount()
//{
//	return (long)groupPlayer.size();
//}

void CLobby::OnMessage(unsigned __int64 sessionId, CPacket* packet)
{
	WORD type;
	/*if (packet->Decoding() == 0)
		DebugBreak();*/

	*packet >> type;

	switch (type)
	{
	case 1001: // en_PACKET_CS_GAME_REQ_LOGIN
		//wprintf(L"en_PACKET_CS_GAME_REQ_LOGIN\n");
		RequestLogin(sessionId, packet);
		//ExitGroup(sessionId);
		break;

	default:
		DebugBreak();
		break;
	}
}

void CLobby::OnMessage(unsigned __int64 sessionId, CPacket* packetArray[], int size)
{
	WORD type;

	//size = min(size, 200);
	unsigned __int64 accountNo;
	bool flag = 0;
	for (int i = 0; i < size; ++i)
	{

		*packetArray[i] >> type;

		switch (type)
		{
		case 1001: // en_PACKET_CS_GAME_REQ_LOGIN
			//wprintf(L"en_PACKET_CS_GAME_REQ_LOGIN\n");
			RequestLogin(sessionId, packetArray[i]);
			//ExitGroup(sessionId);
			break;

		default:
		{
			*packetArray[i] >> accountNo;
			DebugBreak();
			return;
		}
		}
	}

	netLib->PostJob(sessionId);
}


void CLobby::RequestLogin(unsigned __int64 sessionId, CPacket* packetPtr)
{
	INT64 accountNo;
	char sessionKey[64];
	int version;

	// data 뽑기.
	*packetPtr >> accountNo;
	packetPtr->GetData(sessionKey, 64);
	*packetPtr >> version;

	// 전송...
	packetPtr->Clear();
	WORD type = 1002;
	version = 1;
	BYTE status = version;
	*packetPtr << type << status << accountNo;



	auto accountKeyIt = accountKey.find(accountNo); // accountNo, sessionId
	if (accountKeyIt != accountKey.end())
	{
		version = 2;
		DebugBreak();
	}
	auto groupPlayerIt = groupPlayer.find(sessionId);
	if (groupPlayerIt == groupPlayer.end())
	{
		version = 3;
		DebugBreak();
	}

	// groupPlayer.insert({});
	SPlayer* player = groupPlayerIt->second;
	player->accountNo = accountNo;
	if (player->accountNo != accountNo)
		DebugBreak();
	
	accountKey.insert({ accountNo , sessionId }); // accountNo, sessionId

	// Ptr 이용해서 대신 보내기.
	// 
	netLib->SendEnqueue(sessionId, packetPtr);
	//netLib->SendMessages(player->sessionId, packetPtr); // TODO: 05-02
	packetPtr->subRef();
	//wprintf(L"mode:%d\n", )
	
	version = FindIndex(sessionId);
	ExitGroup(sessionId, 3);
	PushIndex(version);
	netLib->ChangeRoom(sessionId);
}

// ============================================

//CField::CField()
///*: CGroup(1),
//playerCount(0)*/
//{
//	//netLib = reinterpret_cast<CGameServerInterface*>(ptr);
//}


CField::CField(void* ptr)
	: CGroup(2, 4, 20)
{
	netLib = reinterpret_cast<CNetLibrary*>(ptr);
	gameFace = reinterpret_cast<CGameServerInterface*>(ptr);
}
unsigned __int64 testArray[10000];
unsigned __int64 testArrayIdx;
unsigned __int64 testArrayIdx2;
unsigned __int64 testArrayIdx3;
unsigned __int64 testArray2[10000];
//
void CField::OnClientGroupJoin(unsigned __int64 sessionId)
{
	SPlayer* newPlayer;	// TODO
	// -----

	gameFace->SRWSLock();
	auto it = gameFace->playersMap.find(sessionId);
	newPlayer = it->second;
	gameFace->SRWSUnLock();

	/*unsigned __int64 idx = InterlockedIncrement(&myIdx);
	idx %= 100000;*/
	auto test = accountKey.find(newPlayer->accountNo);
	if (test != accountKey.end())
	{
		unsigned __int64 a = InterlockedIncrement(&testArrayIdx);
		a %= 10000;
		testArray[a] = test->second;
		testArray2[a] = sessionId;
		accountKey.erase(newPlayer->accountNo);
		a = test->second;
		groupPlayer.erase(a);
		netLib->Disconnect2(a);
	}
	accountKey.insert({ newPlayer->accountNo, sessionId });
	/*auto test = accountKey.insert({ newPlayer->accountNo, sessionId });
	if (test.second == false)
	{
		
		DebugBreak();
		
	}*/
	
	groupPlayer.insert({ sessionId, newPlayer });
	/*asdf[idx].type = 1;
	asdf[idx].sessionId = sessionId;
	asdf[idx].accountNo = newPlayer->accountNo;
	asdf[idx].ptr = newPlayer;*/
}

void CField::OnClientGroupExit(unsigned __int64 sessionId)
{
	SPlayer* player;

	auto it = groupPlayer.find(sessionId);
	if (it == groupPlayer.end())
	{
		return;
	}

	player = it->second;


	/*unsigned __int64 idx = InterlockedIncrement(&myIdx);
	idx %= 100000;
	asdf[idx].type = 2;
	asdf[idx].sessionId = sessionId;
	asdf[idx].accountNo = player->accountNo;
	asdf[idx].ptr = player;*/

	size_t count = accountKey.erase(player->accountNo);
	if (count == 0) // 0 가능
		InterlockedIncrement(&testArrayIdx2);;

	count = groupPlayer.erase(sessionId);
	if (count == 0)
		InterlockedIncrement(&testArrayIdx3);;
}

//long CLobby::GetGroupPlayerCount()
//{
//	return playerCount;
//}
//
//long CLobby::GetGroupPlayerMapCount()
//{
//	return (long)groupPlayer.size();
//}

void CField::Echo(unsigned __int64 sessionId, CPacket* packet)
{
	//CProfiler Echo(L"Field_Echo");
	//SPlayer* player;
	WORD type;
	INT64 accountNo;
	LONGLONG sendTick;
	// -----

	//ProfileBegin(L"EchoFind");
	
	/*auto it = groupPlayer.find(sessionId);
	if (it == groupPlayer.end())
		DebugBreak();

	player = it->second;*/

	*packet >> accountNo >> sendTick;

	type = 5001;

	packet->Clear();
	*packet << type << accountNo << sendTick;
	

	// Player로 바꿔야 함.
	//netLib->SendMessages(sessionId, packet);
	/*if (player->sessionId != sessionId)
		DebugBreak();*/
	//netLib->SendEnqueue(player->sessionId, packet);
	type = netLib->SendEnqueue(sessionId, packet);
	
	// player가 들고 있다가 
	packet->subRef();
	return ;
}

void CField::OnMessage(unsigned __int64 sessionId, CPacket* packet)
{
	WORD type;

	/*if (packet->Decoding() == 0)
		DebugBreak();*/

	*packet >> type;

	switch (type)
	{
	case 5000: // en_PACKET_CS_GAME_REQ_ECHO
		//wprintf(L"CField::en_PACKET_CS_GAME_REQ_ECHO\n");
		Echo(sessionId, packet);
		break;

	case 5001: // en_PACKET_CS_GAME_RES_ECHO (RES임 Error)
		DebugBreak();
		break;

	case 5002: // en_PACKET_CS_GAME_REQ_HEARTBEAT (사용 X)
		wprintf(L"CField::en_PACKET_CS_GAME_REQ_HEARTBEAT\n");
		DebugBreak();
		break;
	default:
		DebugBreak();
		break;
	}
}

void CField::OnMessage(unsigned __int64 sessionId, CPacket* packetArray[], int size)
{
	WORD type;

	// size = min(size, 200);

	for (int i = 0; i < size; ++i)
	{
		*packetArray[i] >> type;

		switch (type)
		{
		case 5000: // en_PACKET_CS_GAME_REQ_ECHO
			//wprintf(L"CField::en_PACKET_CS_GAME_REQ_ECHO\n");
			Echo(sessionId, packetArray[i]);
			break;

		case 5001: // en_PACKET_CS_GAME_RES_ECHO (RES임 Error)
			DebugBreak();
			break;

		case 5002: // en_PACKET_CS_GAME_REQ_HEARTBEAT (사용 X)
			wprintf(L"CField::en_PACKET_CS_GAME_REQ_HEARTBEAT\n");
			DebugBreak();
			break;
		default:
			DebugBreak();
			break;
		}
		
	}
	netLib->PostJob(sessionId);
}
