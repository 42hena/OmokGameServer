#include <stdio.h>
#include <process.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <unordered_map>
#include <string>
#include <Windows.h>


#include "CUser.h"
#include "CRoom.h"

#include "CPacket.h"

#include "CRBuffer.h"

#include "CLFStack.h"
#include "CLFQueue.h"
#include "Session.h"
#include "CBucketPool.h"

#include "CNetworkClientLib.h"
#include "CMonitorChatClient.h"
#include "CNetLibrary.h"

CNetLibrary::CNetLibrary()
	: hIOCP(nullptr),
	sessionID(0),
	sessionTotalCount(0), 
	acceptCount(0), 
	recvCount(0), 
	sendCount(0), 
	listenSocket(INVALID_SOCKET),
	indexPool(20000)
{
	int code;

	hJobEvent = CreateEvent(nullptr, false, 0, nullptr);
	if (hJobEvent == nullptr)
	{
		wprintf(L"Create Job Event Fail\n");
		DebugBreak();
	}

	code = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (code != NO_ERROR)
	{
		code = GetLastError();
		wprintf(L"WSAStartup Fail[%d]\n", code);
		DebugBreak();
	}
	for (code = 19999; code >= 0; --code)
	{
		indexPool.Push(code);
	}
}

CNetLibrary::~CNetLibrary()
{
	WSACleanup();
}

int CNetLibrary::FindArrayIndex(unsigned __int64 sessionID)
{
	return sessionID >> 49;
}
SSession* CNetLibrary::FindSession(unsigned __int64 sessionID)
{
	return &sessionArray[FindArrayIndex(sessionID)];
}

void CNetLibrary::ReleaseSession(unsigned __int64 sessionID)
{
	SOCKET exitSocket;
	SSession* session;
	const long flag = 0x0001'0000;
	long refCountRet;
// -----

	session = FindSession(sessionID);
	
	//InterlockedExchange(&sessionArray[index].flag, 0);
	refCountRet = InterlockedCompareExchange(&session->IOCount, flag, 0L);
	//ret = InterlockedOr(&session->IOCount, flag);
	if (refCountRet != 0)
	{
		return;
	}

	//session->Record(session->sessionID, refCountRet, 51);
	// 삭제 진행.
	exitSocket = INVALID_SOCKET;
	exitSocket = session->socket;
	session->socket = INVALID_SOCKET;
	closesocket(exitSocket);
	

	int ret = session->sendQ.GetSize();
	CPacket* packet;
	for (int i = 0; i < ret; ++i)
	{
		int rrr = session->sendQ.Dequeue(packet);
		if (rrr == -1)
			DebugBreak();
		packet->subRef();
	}
	
	/*uintptr_t now = InterlockedIncrement(&sssIdx);
	now %= 131072;
	sssarray[now].sessionID = session->sessionID;
	sssarray[now].socket = exitSocket;
	sssarray[now].port = session->port;
	memcpy(sssarray[now].clientIP, session->clientIP, sizeof(session->clientIP));
	
	sssarray[now].type = 2;*/



	indexPool.Push(FindArrayIndex(sessionID));
	//InterlockedDecrement((long*)&sessionCount);
	OnClientLeave(sessionID);
}

void CNetLibrary::RecvPost(SSession* session)
{
	WSABUF wsabuf[2];
	DWORD flag, bufCount;
	int code;
	long currentIOCount;
// -----

	if (InterlockedOr(&session->disconnectFlag, 0) == 1)
		return;

	bufCount = 1;
	wsabuf[0].buf = session->recvQ.GetRearBufferPtr();
	wsabuf[0].len = session->recvQ.DirectEnqueueSize();
	if (session->recvQ.GetFreeSize() != wsabuf[0].len)
	{
		if (session->recvQ.GetFreeSize() <= wsabuf[0].len)
			Sleep(10);
		++bufCount;
		wsabuf[1].buf = session->recvQ.GetBufferPtr();
		wsabuf[1].len = (session->recvQ.GetFreeSize() - wsabuf[0].len);
	}

	//ZeroMemory(&session->recvOverlap.overlap, sizeof(OVERLAPPED));
	ZeroMemory(&session->recvOverlap.overlap, sizeof(session->recvOverlap.overlap));

	flag = 0;
	long ref = InterlockedIncrement(&session->IOCount);
	//session->Record(session->sessionID, ref, 21);
	//code = WSARecv(session->socket, wsabuf, bufCount, nullptr, &flag, (OVERLAPPED*)&session->recvOverlap, nullptr);
	if (WSARecv(session->socket, wsabuf, bufCount, nullptr, &flag, (OVERLAPPED*)&session->recvOverlap, nullptr) == SOCKET_ERROR)
	{
		code = GetLastError();
		if (code == ERROR_IO_PENDING)
		{
			if (session->disconnectFlag)
			{
				CancelIoEx((HANDLE)session->socket, &session->recvOverlap.overlap);
			}
		}
		else
		{
			if (code == 10038 || code == 10053 || code == 10054)
				;
			else
				wprintf(L"RecvPost Fail[code:%d]\n", code);
			ref = InterlockedDecrement(&session->IOCount);
			//session->Record(session->sessionID, ref, 22);
			if (ref == 0)
			{
				ReleaseSession(session->sessionID);
			}
			return;
		}
	}
	InterlockedIncrement(&recvCount);
}

void CNetLibrary::SendPost(SSession* session)
{
	CPacket* ptr;
	WSABUF wsabuf[200];
	DWORD flag, bufCount;
	int code;
	int useSize;
// -----

	if (InterlockedExchange(&session->sendFlag, 1) == 0)
	{
		useSize = session->sendQ.GetSize();

		if (useSize > 0)
		{
			flag = 0;
			bufCount = 0;
			useSize = min(useSize, 200);
			if (useSize == 200)
			{
				while (1)
				{
					wprintf(L"qwer Full\n");
				}
			}
			for (bufCount = 0; bufCount < useSize; ++bufCount)
			{
				session->sendQ.Dequeue(ptr);
				wsabuf[bufCount].buf = ptr->GetBufferPtr();
				wsabuf[bufCount].len = ptr->GetDataSize() + 5;
				session->sendingArray[bufCount] = ptr;
			}
			session->sendCount = bufCount;
			//InterlockedExchange(&session->sendCount, bufCount);
			
			//ZeroMemory(&session->sendOverlap.overlap, sizeof(session->sendOverlap.overlap));
			ZeroMemory(&session->sendOverlap.overlap, sizeof(OVERLAPPED));

			// IO_Count
			long ref = InterlockedIncrement(&session->IOCount);
			//session->Record(session->sessionID, ref, 11);
			//code = WSASend(session->socket, wsabuf, bufCount, nullptr, flag, (OVERLAPPED*)&session->sendOverlap, nullptr);
			if (WSASend(session->socket, wsabuf, bufCount, nullptr, flag, (OVERLAPPED*)&session->sendOverlap, nullptr) == SOCKET_ERROR)
			{
				code = GetLastError();
				if (code != ERROR_IO_PENDING) //997이면 IO_PENDING
				{
					// 처리못한 애들 처리
					for (bufCount = 0; bufCount < useSize; ++bufCount)
					{
						session->sendingArray[bufCount]->subRef();
					}

					if (code == 10038 || code == 10053 || code == 10054)
						;
					else
						wprintf(L"SendPost Fail[code:%d]\n", code);
					//currentIOCount = InterlockedDecrement(&session->IOCount);

					ref = InterlockedDecrement(&session->IOCount);
					//session->Record(session->sessionID, ref, 12);
					if (ref == 0)
					{
						ReleaseSession(session->sessionID);
					}
					return;
				}
			}
			InterlockedIncrement(&sendCount);
		}
		else
		{
			long ref = InterlockedIncrement(&session->IOCount);
			//session->Record(session->sessionID, ref, 13);
			if (useSize < 0)
			{
				DebugBreak();
			}
			PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)session, &session->sendOverlap.overlap);
			//InterlockedExchange(&session->sendFlag, 0);
		}
	}
}

unsigned int CNetLibrary::AcceptThread(LPVOID param)
{
	int errCode;
	SOCKADDR_IN addr;
	SOCKET clientSocket;
	int addrlen;
	SSession* newSession;
	const char* ip = "0.0.0.0";
	CNetLibrary* ptr = reinterpret_cast<CNetLibrary*>(param);
	uintptr_t nextID;
	long retRefCount;
// -----

	ptr->listenSocket = INVALID_SOCKET;

	ptr->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ptr->listenSocket == INVALID_SOCKET)
	{
		errCode = GetLastError();
		wprintf(L"socket Fail[%d]\n", errCode);
		return (0);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(12001); // TODO:
	inet_pton(AF_INET, ip, &(addr.sin_addr));

	errCode = bind(ptr->listenSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
	if (errCode == SOCKET_ERROR)
	{
		errCode = GetLastError();
		wprintf(L"bind Fail[%d]\n", errCode);
		return (0);
	}

	LINGER ling;
	ling.l_onoff = 1;
	ling.l_linger = 0;
	DWORD tt = setsockopt(ptr->listenSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));
	if (tt == SOCKET_ERROR)
	{
		DebugBreak();
	}
	
	int ss = 0;
	tt = setsockopt(ptr->listenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&ss, sizeof(ss));
	if (tt == SOCKET_ERROR)
	{
		DebugBreak();
	}

	listen(ptr->listenSocket, SOMAXCONN_HINT(65535));
	addrlen = sizeof(SOCKADDR_IN);
	
	
	// Accept Part
	for( ; ; )
	{
		clientSocket = accept(ptr->listenSocket, (SOCKADDR*)&addr, &addrlen);
		
		if (clientSocket == INVALID_SOCKET)
		{
			errCode = GetLastError();
			wprintf(L"accept Fail[%d]\n", errCode);
			if (errCode == WSAEINTR) // 10004 함수 호출이 중단
			{
				wprintf(L"Cancel Calling Accept\n");
			}
			else if (errCode == WSAENOTSOCK) // 10038 소켓이 아닌 경우
			{
				wprintf(L"Non Socket\n");
			}
			break;
		}

		int index = -1;

		bool rrr = ptr->indexPool.Pop(index);
		if (!rrr)
		{
			closesocket(clientSocket);
			continue;
		}
		

		

		nextID = ++(ptr->sessionID) | ((unsigned long long)index << 49);
		//newSession = &ptr->sessionArray[index].session;
		newSession = &ptr->sessionArray[index];

		InetNtopW(AF_INET, &(addr.sin_addr), newSession->clientIP, INET_ADDRSTRLEN);
		newSession->port = ntohs(addr.sin_port);
		newSession->socket = clientSocket;
		/*newSession->sessionID = ++(ptr->sessionID);
		newSession->sessionID |= ((unsigned long long)index << 50);*/
		newSession->recvQ.ClearBuffer();
		newSession->sendFlag = 0;
		newSession->disconnectFlag = 0;
		
		
		//newSession->len = 0;


		//newSession->sessionID = nextID;
		if (newSession->sendQ.GetSize() > 0)
			DebugBreak();
		uintptr_t aa =InterlockedExchange(&newSession->sessionID, nextID);
		retRefCount = InterlockedAdd(&newSession->IOCount, 0xffff'0001);
		if (retRefCount <= 0)
			DebugBreak();
		//newSession->Record(newSession->sessionID, retRefCount, 61);
		
		/*uintptr_t now = InterlockedIncrement(&sssIdx);
		now %= 131072;
		sssarray[now].sessionID = nextID;
		sssarray[now].socket = clientSocket;
		sssarray[now].port = newSession->port;
		InetNtopW(AF_INET, &(addr.sin_addr), sssarray[now].clientIP, INET_ADDRSTRLEN);
		sssarray[now].type = 1;*/

		InterlockedIncrement(&ptr->sessionTotalCount);
		InterlockedIncrement(&ptr->acceptCount);

		ptr->OnClientJoin(newSession->sessionID);

		// IOCP와 socket 연결
		CreateIoCompletionPort((HANDLE)clientSocket, ptr->hIOCP, (ULONG_PTR)newSession, 0);
		ptr->RecvPost(newSession);
		retRefCount = InterlockedDecrement(&newSession->IOCount);
		if (retRefCount == 0)
		{
			ptr->ReleaseSession(newSession->sessionID);
		}
	}
	wprintf(L"Accept Thread Normal Exit\n"); // 파일로 다 뺄까?

	return (0);
}

unsigned int CNetLibrary::IOCPWorkerThread(LPVOID param)
{
	PULONG key;
	DWORD byte;
	SExteneOverlap* overlap;
	int errCode;
	int code;
	const int HeaderSize = 5;
	CPacket *packet;
	WORD size;
	long currentIOCount;
	SSession* session;
	CNetLibrary* ptr = reinterpret_cast<CNetLibrary*>(param);
	uintptr_t saveID;
// ------

	for (;;)
	{
		key = nullptr;
		byte = 0;
		overlap = nullptr;
		code = GetQueuedCompletionStatus(ptr->hIOCP, &byte, (PULONG_PTR)&key, (LPOVERLAPPED*)&overlap, INFINITE);
		// 종료 처리
		if (byte == 0 && key == nullptr && overlap == nullptr)
		{
			if (code == false)
			{
				code = GetLastError();
				if (code == ERROR_ABANDONED_WAIT_0)
				{
					wprintf(L"Before call GQCS, IOCP Handle is closed\n");
				}
				if (code == ERROR_INVALID_HANDLE)
				{
					wprintf(L"INVALID HANDLE\n");
				}
				//PostQueuedCompletionStatus(ptr->hIOCP, 0, 0, nullptr);
				DebugBreak();
			}
		}

		session = (SSession*)key;

		if (overlap == nullptr)
		{
			if (byte == 0)
			{
				code = GetLastError();
				if (code == 64) // rst
					;
				if (session->sendQ.GetSize() > 0)
					ptr->SendPost(session);
				currentIOCount = InterlockedDecrement(&session->IOCount);
				if (currentIOCount == 0)
				{
					ptr->ReleaseSession(session->sessionID);
				}
				continue;
			}
			else
				DebugBreak();
		}
		

		if (code == false)
		{
			code = GetLastError();
			if (code == 64) // rst
				; // session->Record(session->sessionID, 0xffffffff, 90);
			else if (code == 997) // cancel IO
				;
			else
			{
				wprintf(L"Code: %d id:%lld sock:%lld\n", code, session->sessionID, session->socket);
				Sleep(INFINITE);
			}
		}

		//wprintf(L"byte:%d, key:%p, ovarlap:%p\n", byte, key, overlap);
			//if (byte != code) 짤려서 올 수 있음.
			//		DebugBreak();

		if (overlap->mode == 0) // recv
		{
			int ret = session->recvQ.MoveRear(byte);
			if (ret != byte)
				DebugBreak();

			code = session->recvQ.GetUseSize();
			saveID = session->sessionID;

			while (code >= HeaderSize)
			{
				//session->lastTime = timeGetTime();
				packet = CPacket::Alloc();
				if (packet->GetRefCount() != 0)
					DebugBreak();

				packet->AddRef();
				if (packet->GetRefCount() != 1)
					DebugBreak();

				int peekSize = session->recvQ.Peek(packet->GetBufferPtr(), HeaderSize);
				if (peekSize != 5)
				{
					DebugBreak();
					break;
				}

				packet->GetHeader((char*)&size, 1, 2);

				if (size > 512)
				{
					DebugBreak();
					ptr->Disconnect(session->sessionID);
					packet->subRef();
					break;
				}

				if (HeaderSize + size > code) //  vs ringbuffer
				{
					packet->subRef();
					break;
				}


				int ret = session->recvQ.MoveFront(HeaderSize);
				if (ret != HeaderSize)
					DebugBreak();
				ret = session->recvQ.Dequeue(packet->GetBufferPtr() + HeaderSize, size);
				if (ret != size)
					DebugBreak();

				// packet move
				ret = packet->MoveWritePos(size);
				if (ret != size)
				{
					DebugBreak();
					break;
				}
				// ptr->OnMessage(session->sessionID, &packet);
				if (saveID != session->sessionID)
					DebugBreak();
				ptr->OnMessage(session->sessionID, packet);
				code -= ret + 5;
			}

			ptr->RecvPost(session);
		}
		if (overlap->mode == 1) // send
		{
			// send통지 처리
			for (int i = 0; i < session->sendCount; ++i)
			{
				session->sendingArray[i]->subRef();
			}
			session->sendCount = 0;
			int ret = InterlockedExchange(&session->sendFlag, 0);
			if (session->sendQ.GetSize() > 0)
				ptr->SendPost(session);
		}
		
		currentIOCount = InterlockedDecrement(&session->IOCount);
		//long ref = InterlockedIncrement(&session->IOCount);
		//session->Record(session->sessionID, currentIOCount, 31);
		if (currentIOCount == 0)
		{
			ptr->ReleaseSession(session->sessionID);
		}
	}


	wprintf(L"IOCP WorkerThread Exit\n");
	return (0);
}
bool CNetLibrary::ExcuteServer(
	const WCHAR* ip, USHORT port, int workerThreadCount, int runningThreadCount,
	bool nagleOn, int maxSessionCount)
{
	int errCode;
	int idx;
	HANDLE hAcceptThread;
	HANDLE hWorkerThread[4];
// -----

	wprintf(L"IP: %s\n"
		"PORT: %d\n"
		"WorkerCount:%d MaxrunningCount:%d\n", ip, port, workerThreadCount, runningThreadCount);

	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, runningThreadCount);

	hAcceptThread = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);
	if (hAcceptThread == nullptr)
	{
		errCode = GetLastError();
		wprintf(L"hAcceptThread Fail %d\n", errCode);
		return (false);
	}

	for (idx = 0; idx < workerThreadCount; ++idx)
	{
		hWorkerThread[idx] = (HANDLE)_beginthreadex(nullptr, 0, IOCPWorkerThread, this, 0, nullptr);
		if (hWorkerThread[idx] == nullptr)
		{
			errCode = GetLastError();
			wprintf(L"hWorkerThread Fail % d\n", errCode);
			return (false);
		}
	}

	return (true);
}

bool CNetLibrary::Disconnect(unsigned __int64 sessionID)
{
	SSession* session;
	SOCKET sock;
	int index;
	long refCount;
// ------


	DebugBreak();

	session = FindSession(sessionID);

	refCount = InterlockedIncrement(&session->IOCount);
	if (refCount >= 0x0001'0000)
	{
		refCount = InterlockedDecrement(&session->IOCount);
		if (refCount <= 0)
			DebugBreak();
		return false;
	}
	if (refCount == 0)
		DebugBreak();

	// session ID 다를 때
	if (sessionID != session->sessionID)
	{
		refCount = InterlockedDecrement(&session->IOCount);
		if (refCount <= 0)
			DebugBreak();
		return false;
	}

	//InterlockedIncrement(&disIdx);
	// flag 변환	
	long flag = InterlockedExchange(&session->disconnectFlag, 1);
	if (flag == 0)
	{
		//CancelIoEx((HANDLE)session->socket, &session->recvOverlap.overlap);
		CancelIoEx((HANDLE)session->socket, nullptr);
		refCount = InterlockedDecrement(&session->IOCount);
		if (refCount == 0)
			ReleaseSession(sessionID);
		return (true);
	}

	refCount = InterlockedDecrement(&session->IOCount);
	if (refCount == 0)
		ReleaseSession(sessionID);

	return false;
}

void CNetLibrary::SendMessages(unsigned __int64 sessionID, CPacket* packet)
{
	SSession* session;

	BYTE code = 0x77;
	unsigned short len = 0;
	BYTE rk = 1;
	DWORD checkSum = 0;
	long long data;
	int index;
	long refCountRet;
	// -----

	/*index = (sessionID >> 50);
	session = &sessionArray[index];*/

	session = FindSession(sessionID);
	
	if (session->disconnectFlag)
		return;

	refCountRet = InterlockedIncrement(&session->IOCount);
	if (refCountRet >= 0x0001'0000)
	{
		refCountRet = InterlockedDecrement(&session->IOCount); // 이미 지우는 작업을 시작.
		if (refCountRet <= 0)
			DebugBreak();
		return;
	}
	if (refCountRet == 0)
		DebugBreak();

	
	if (sessionID != InterlockedOr(&session->sessionID, 0))
	{
		refCountRet = InterlockedDecrement(&session->IOCount); // 이미 지우는 작업을 시작.
		if (refCountRet <= 0)
			DebugBreak();
		return;
	}

	if (packet->encodeFlag == false)
	{
		len = packet->GetDataSize();
		packet->SetHeader((char*)&len, 1, 2);
		InterlockedExchange(&packet->encodeFlag, 1);
	}

	// sessionID, packet
	packet->AddRef();

	session->sendQ.Enqueue(packet);

	//SendPost(session);
	
	// TODO:Add
	if (!InterlockedOr(&session->sendFlag, 0))
	{
		refCountRet = InterlockedIncrement(&session->IOCount);

		PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)session, nullptr);
	}

	refCountRet = InterlockedDecrement(&session->IOCount);
	
	//session->Record(session->sessionID, refCountRet, 42);
	if (refCountRet >= 0x0001'0000 || refCountRet < 0)
		DebugBreak();
	if (refCountRet == 0)
	{
		ReleaseSession(sessionID);
	}
}

void CNetLibrary::Pause()
{

}

// ##################################################

// ##################################################

long CNetLibrary::GetAcceptTPS()
{
	return InterlockedExchange(&acceptCount, 0);
	// return (acceptCount);
}

long CNetLibrary::GetRecvMessageTPS()
{
	return InterlockedExchange(&recvCount, 0);
	//return (recvCount);
}

long CNetLibrary::GetSendMessageTPS()
{
	return InterlockedExchange(&sendCount, 0);
	//return (sendCount);
}

long CNetLibrary::GetSessionCount()
{
	return (sessionTotalCount);
}
