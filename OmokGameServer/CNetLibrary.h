#ifndef __CLASS_NET_LIBRARY__
#define __CLASS_NET_LIBRARY__


struct SData
{
	SData()
		:flag(false)
	{}
	long flag;
	SSession session;
};

class CNetLibrary
{
public:
	// ##################################################
	// #                기본     함수들                 #
	// ##################################################
	CNetLibrary();
	virtual ~CNetLibrary();

	// 사용하지 않는 함수들
	CNetLibrary(const CNetLibrary& copy) = delete;				// c++98이면 private로 바꾸기
	CNetLibrary& operator=(const CNetLibrary& copy) = delete;	// c++98이면 private로 바꾸기

public:
	// ##################################################
	// #                외부 노출 함수                  #
	// ##################################################
	bool ExcuteServer(const WCHAR* ip, USHORT port, int workerThreadCount, int runningThreadCount, bool nagleOn, int maxSessionCount);
	void Pause();	// 미구현

	void RecvPost(SSession* session);
	void SendPost(SSession* session);
	virtual void ReleaseSession(unsigned __int64 sessionID);
	int FindArrayIndex(unsigned __int64 sessionID);
	SSession* FindSession(unsigned __int64 sessionID);

	bool Disconnect(unsigned __int64 sessionID);
	virtual void SendMessages(unsigned __int64 sessionID, CPacket* packet);

	virtual bool OnConnectionRequest(const WCHAR* IPAddress, USHORT port) = 0;	// 미구현
	virtual void OnClientJoin(unsigned __int64 sessionID) = 0;
	virtual void OnClientLeave(unsigned __int64 sessionID) = 0;
	virtual void OnMessage(unsigned __int64 sessionID, CPacket* packet) = 0;
	virtual void OnError(int errorcode, const WCHAR* msg) = 0;					// 미구현

	// ##################################################
	// #                모니터링 요소들                 #
	// ##################################################
	long GetAndInitAcceptTPS();
	long GetAndInitRecvMessageTPS();
	long GetAndInitSendMessageTPS();
	void CountUpAcceptTPS();
	void CountUpRecvTPS();
	void CountUpSendTPS();
	long GetSessionCount();

	HANDLE GetJobEvent()
	{
		return hJobEvent;
	}

	// ##################################################
	// #                쓰레드   정보들                 #
	// ##################################################
	static unsigned int __stdcall AcceptThread(LPVOID param);
	static unsigned int __stdcall IOCPWorkerThread(LPVOID param);
		
	long GetIndexPoolSize(void)
	{
		return indexPool.GetSize();
	}
	long GetIndexPoolAllocSize(void)
	{
		return indexPool.GetPoolAllocSize();
	}

	long GetIndexPoolUseSize(void)
	{
		return indexPool.GetUseSize();
	}

	SOCKET	listenSocket;
private:
	HANDLE	hIOCP;
	WSADATA wsa;
	//SData sessionArray[16384];

	SSession sessionArray[20000];

	unsigned long long sessionID;

	HANDLE hJobEvent;
	CStack<int> indexPool;
	//CMemoryPool<int> indexPool{16384, false};

private:
	alignas (64) long _sessionCount;
	alignas (64) long _acceptCount;
	alignas (64) long _recvCount;
	alignas (64) long _sendCount;
};

#endif
