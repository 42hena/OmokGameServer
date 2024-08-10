#pragma once

struct SExteneOverlap {
	SExteneOverlap(int mode)
		: mode(mode), overlap{0}
	{
	}
	OVERLAPPED overlap;
	int mode;
};

struct SSessionProfile
{
	uintptr_t sid;
	long IOcnt;
	int type;
};

struct SSession
{
	SSession()
		: IOCount(0x00010000), 
		recvOverlap(0), 
		sendOverlap(1), 
		sendFlag(0), 
		sessionID(0), 
		socket(INVALID_SOCKET),
		disconnectFlag(0)
	{
		// TODO: 03-28
		clientIP[0] = 0;
		port = 0;
		sendingArray[0] = nullptr;
		sendCount = 0;


		InitializeSRWLock(&QSRWLock);

	}
	~SSession()
	{}
	SOCKET socket;
	WCHAR clientIP[22];
	USHORT port;
	SExteneOverlap recvOverlap;
	SExteneOverlap sendOverlap;
	CRBuffer recvQ;
	CLFQueue<CPacket*> sendQ;
	unsigned __int64 sessionID;
	CPacket* sendingArray[200];
	long sendCount;
	long IOCount;
	long sendFlag;
	long disconnectFlag;
	SRWLOCK QSRWLock;

	/*SSessionProfile pro[1000];
	alignas (64) uintptr_t idx;*/

	/*void Record(uintptr_t sid, int iocount, int type)
	{
		uintptr_t now = InterlockedIncrement(&idx);
		now %= 1000;
		pro[now].sid = sid;
		pro[now].IOcnt = IOCount;
		pro[now].type = type;
	}*/
};
