#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <process.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "CRBuffer.h"
#include "CLFQueue.h"

#include "CPacket.h"
#include "Session.h"
#include "CNetworkClientLib.h"


CNetworkClientLib::CNetworkClientLib()
	: 
	hIOCP(0),
	flag(0),
	hConnectThread(0),
	hWorkerThread(0),
	hSendThread(0),
	serverSocket(INVALID_SOCKET),
	serverIpAddress(nullptr),
	serverPort(0),
	totalConnectCount(0),
	totalRecvCount(0),
	totalSendCount(0),
	connectTPS(0),
	recvTPS(0),
	sendTPS(0)
{
	hExitEvent = CreateEvent(nullptr, true, 0, nullptr);
	if (hExitEvent == nullptr)
	{
		DebugBreak();
	}

	hConnectEvent = CreateEvent(nullptr, false, 0, nullptr);
	if (hConnectEvent == nullptr)
	{
		DebugBreak();
	}

	wprintf(L"CNetworkClientLib constructor\n");
}

CNetworkClientLib::~CNetworkClientLib()
{
	wprintf(L"CNetworkClientLib destructor\n");
}

void CNetworkClientLib::ReleaseSession()
{
	SOCKET exitSocket;
	const long flag = 0x0001'0000;
	long refCountRet;
	CPacket* packet;
// -----

	refCountRet = InterlockedCompareExchange(&session.IOCount, flag, 0L);
	if (refCountRet != 0)
		return;

	// 삭제 진행.
	exitSocket = session.socket;
	session.socket = INVALID_SOCKET;
	closesocket(exitSocket);



	int ret = session.sendQ.GetSize();
	if (ret > 0)
	{
		for (int i = 0; i < ret; ++i)
		{
			int rrr = session.sendQ.Dequeue(packet);
			if (rrr == -1)
				DebugBreak();
			packet->subRef();
		}
	}
	OnLeaveServer();
}

void CNetworkClientLib::RecvPost(SSession* session)
{
	WSABUF wsabuf[2];
	DWORD flag, bufCount;
	int code;
	long currentIOCount;
	// -----

		// Reserve
	if (InterlockedOr(&session->disconnectFlag, 0) > 0)
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
	InterlockedIncrement(&session->IOCount);
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
			if (InterlockedDecrement(&session->IOCount) == 0)
			{
				ReleaseSession();
			}
			return;
		}
	}
	totalRecvCount++;
	InterlockedIncrement(&recvTPS);
}

void CNetworkClientLib::SendPost(SSession* session)
{
	CPacket* packetPtr;
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
			for (bufCount = 0; bufCount < useSize; ++bufCount)
			{
				session->sendQ.Dequeue(packetPtr);
				wsabuf[bufCount].buf = packetPtr->GetBufferPtr();
				wsabuf[bufCount].len = packetPtr->GetDataSize() + 5;
				session->sendingArray[bufCount] = packetPtr;
			}
			session->sendCount = bufCount;

			ZeroMemory(&session->sendOverlap.overlap, sizeof(OVERLAPPED));

			// IO_Count
			InterlockedIncrement(&session->IOCount);
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
					if (InterlockedDecrement(&session->IOCount) == 0)
					{
						ReleaseSession();
					}
					return;
				}
			}
			totalSendCount++;
			InterlockedIncrement(&sendTPS);
		}
		else
		{
			InterlockedExchange(&session->sendFlag, 0);
			if (useSize < 0)
			{
				DebugBreak();
			}
		}
	}
}

unsigned int CNetworkClientLib::ConnectThread(LPVOID param)
{
	CNetworkClientLib* thisPtr;
	int code;
	LINGER ling;
	struct sockaddr_in serverAddr;
// -----
	
	// class의 this ptr 변환
	thisPtr = reinterpret_cast<CNetworkClientLib*>(param);
	
	thisPtr->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (thisPtr->serverSocket == INVALID_SOCKET)
	{
		code = GetLastError();
		wprintf(L"socket Fail[%d]\n", code);
		DebugBreak();
	}
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(thisPtr->serverPort);
	InetPton(AF_INET, thisPtr->serverIpAddress, &serverAddr.sin_addr);


	ling.l_onoff = 1;
	ling.l_linger = 0;
	if (setsockopt(thisPtr->serverSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling)) == SOCKET_ERROR)
	{
		DebugBreak();
	}

	code = 0;
	if (setsockopt(thisPtr->serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&code, sizeof(code)) == SOCKET_ERROR)
	{
		code = GetLastError();
		wprintf(L"code:%d\n", code);
		DebugBreak();
	}

	code = connect(thisPtr->serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (code == SOCKET_ERROR)
	{
		code = GetLastError();
		wprintf(L"Monitor connect Fail%d\n", code);
		return (1);
	}
	SSession* newSession = &thisPtr->session;


	InetNtopW(AF_INET, &(serverAddr.sin_addr), newSession->clientIP, INET_ADDRSTRLEN);
	wprintf(L"ip:%s port:%d\n", newSession->clientIP, newSession->port);
	newSession->port = ntohs(serverAddr.sin_port);
	newSession->socket = thisPtr->serverSocket;
	newSession->sessionID = ++(thisPtr->sessionID);
	newSession->recvQ.ClearBuffer();
	newSession->sendFlag = 0;
	newSession->disconnectFlag = 0;

	code = InterlockedAdd(&newSession->IOCount, 0xffff'0001);
	if (code <= 0)
		DebugBreak();

	InterlockedIncrement(&thisPtr->totalSessionCount);
	InterlockedIncrement(&thisPtr->connectTPS);

	thisPtr->OnEnterJoinServer();
	//ptr->OnClientJoin(newSession->sessionID);

	// IOCP와 socket 연결
	CreateIoCompletionPort((HANDLE)thisPtr->serverSocket, thisPtr->hIOCP, (ULONG_PTR)newSession, 0);
	thisPtr->RecvPost(newSession);
	code = InterlockedDecrement(&newSession->IOCount);
	if (code == 0)
	{
		thisPtr->ReleaseSession();
	}

	return (0);
}

bool CNetworkClientLib::Disconnect()
{
	SOCKET sock;
	long refCount;
// ------

	refCount = InterlockedIncrement(&session.IOCount);
	if (refCount >= 0x0001'0000)
	{
		refCount = InterlockedDecrement(&session.IOCount);
		if (refCount <= 0)
			DebugBreak();
		return false;
	}
	if (refCount == 0)
		DebugBreak();

	long flag = InterlockedExchange(&session.disconnectFlag, 1);
	if (flag == 0)
	{
		CancelIoEx((HANDLE)session.socket, &session.recvOverlap.overlap);
		refCount = InterlockedDecrement(&session.IOCount);
		if (refCount == 0)
			ReleaseSession();
		return (true);
	}

	refCount = InterlockedDecrement(&session.IOCount);
	if (refCount == 0)
		ReleaseSession();

	return false;
}


void CNetworkClientLib::SendMessages(CPacket* packet)
{
	BYTE code = 0x77;
	unsigned short len = 0;
	long refCountRet;
// -----

	if (InterlockedOr(&session.disconnectFlag, 0) > 0)
		return;
	refCountRet = InterlockedIncrement(&session.IOCount);
	if (refCountRet >= 0x0001'0000)
	{
		refCountRet = InterlockedDecrement(&session.IOCount); // 이미 지우는 작업을 시작.
		if (refCountRet <= 0)
			DebugBreak();
		return;
	}
	if (refCountRet == 0)
		DebugBreak();

// -------------------------------

	if (packet->encodeFlag == false)
	{
		len = packet->GetDataSize();
		packet->SetHeader((char*)&code, 0, 1);
		packet->SetHeader((char*)&len, 1, 2);
		packet->encodeFlag = true;
	}

	// sessionID, packet
	packet->AddRef();
	session.sendQ.Enqueue(packet);

	//SendPost(session);

	// TODO:Add
	refCountRet = InterlockedIncrement(&session.IOCount);

	PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)&session, nullptr);
	//PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)&session, &session.sendOverlap.overlap);

	refCountRet = InterlockedDecrement(&session.IOCount);
	if (refCountRet >= 0x0001'0000 || refCountRet < 0)
		DebugBreak();
	if (refCountRet == 0)
	{
		ReleaseSession();
	}
}

bool CNetworkClientLib::TryToConnectServer( const WCHAR* ip, USHORT port,  int workerThreadCount, bool nagleOn )
{
	int code;
// -----

	if (flag)
	{
		wprintf(L"alerady running...\n");
		return false;
	}

	wprintf(L"TryToConnectServer    IP: %s\n""PORT: %d\n""WorkerCount:%d MaxrunningCount:%d\n", ip, port, workerThreadCount, 1);

	if (workerThreadCount > 1)	// Log 사용
	{
		DebugBreak();
		return (false);
	}

	// ip 설정
	int size = wcslen(ip);
	serverIpAddress = new WCHAR[size];
	memcpy(serverIpAddress, ip, size * 2);
	// port 설정.
	serverPort = port;

	// Create IOCP
	if (hIOCP == nullptr)
	{
		hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
	}

	hConnectThread = (HANDLE)_beginthreadex(nullptr, 0, ConnectThread, this, 0, nullptr);
	if (hConnectThread == nullptr)
	{
		code = GetLastError();
		wprintf(L"hConnectThread Fail %d\n", code);
		return (false);
	}

	hWorkerThread = (HANDLE)_beginthreadex(nullptr, 0, IOCPWorkerThread, this, 0, nullptr);
	if (hWorkerThread == nullptr)
	{
		code = GetLastError();
		wprintf(L"hWorkerThread Fail % d\n", code);
		DebugBreak();
		return (false);
	}

	return (true);
}

unsigned int CNetworkClientLib::IOCPWorkerThread(LPVOID param)
{
	PULONG key;
	DWORD byte;
	SExteneOverlap* overlap;
	int errCode;
	int code;
	const int HeaderSize = 5;
	CPacket* packet;
	WORD size;
	long currentIOCount;
	SSession* sessionPtr;
	CNetworkClientLib* thisPtr = reinterpret_cast<CNetworkClientLib*>(param);
	uintptr_t saveID;
	// ------

	for (;;)
	{
		key = nullptr;
		byte = 0;
		overlap = nullptr;
		code = GetQueuedCompletionStatus(thisPtr->hIOCP, &byte, (PULONG_PTR)&key, (LPOVERLAPPED*)&overlap, INFINITE);

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

		sessionPtr = (SSession*)key;

		if (overlap == nullptr)
		{
			if (byte == 0)
			{
				if (sessionPtr->sendQ.GetSize() > 0)
					thisPtr->SendPost(sessionPtr);
				currentIOCount = InterlockedDecrement(&sessionPtr->IOCount);
				if (currentIOCount == 0)
				{
					thisPtr->ReleaseSession();
				}
				continue;
			}
			else
				DebugBreak();
		}
		if (overlap->mode == 0) // recv
		{
			if (sessionPtr->recvQ.MoveRear(byte) != byte)
				DebugBreak();

			code = sessionPtr->recvQ.GetUseSize();

			saveID = sessionPtr->sessionID;

			while (code >= HeaderSize)
			{
				packet = CPacket::Alloc();
				if (packet->GetRefCount() != 0)
					DebugBreak();

				packet->AddRef();
				if (packet->GetRefCount() != 1)
					DebugBreak();

				if (sessionPtr->recvQ.Peek(packet->GetBufferPtr(), HeaderSize) != 5)
				{
					DebugBreak();
					break;
				}

				BYTE netCode;
				packet->GetHeader((char*)&netCode, 0, 1);
				if (netCode != 0x77)
				{
					thisPtr->Disconnect();
					packet->subRef();
					break;
				}
				packet->GetHeader((char*)&size, 1, 2);
				if (HeaderSize + size > code) //  vs ringbuffer
				{
					packet->subRef();
					break;
				}


				int ret = sessionPtr->recvQ.MoveFront(HeaderSize);
				if (ret != HeaderSize)
					DebugBreak();
				ret = sessionPtr->recvQ.Dequeue(packet->GetBufferPtr() + HeaderSize, size);
				if (ret != size)
					DebugBreak();

				// packet move
				ret = packet->MoveWritePos(size);
				if (ret != size)
				{
					DebugBreak();
					break;
				}
				thisPtr->OnMessage(packet);
				code -= ret + 5;
			}

			thisPtr->RecvPost(sessionPtr);
		}
		if (overlap->mode == 1) // send
		{
			// send통지 처리
			if (byte == 0)
			{
				if (code == 0)
					int a = 0;
				code = GetLastError();
				int a = 0;
			}
			if (byte != 1)
			{
				for (int i = 0; i < sessionPtr->sendCount; ++i)
				{
					sessionPtr->sendingArray[i]->subRef();
				}
				sessionPtr->sendCount = 0;
				//InterlockedExchange(&session->sendCount, 0);
				int ret = InterlockedExchange(&sessionPtr->sendFlag, 0); // TODO: 락 뺏을 때
			}


			if (sessionPtr->sendQ.GetSize() > 0)
				thisPtr->SendPost(sessionPtr);
		}

		currentIOCount = InterlockedDecrement(&sessionPtr->IOCount);
		if (currentIOCount == 0)
		{
			thisPtr->ReleaseSession();
		}
	}

	wprintf(L"IOCP WorkerThread Exit\n");
	
	return 0;
}