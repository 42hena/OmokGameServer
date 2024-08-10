#pragma once

class CNetworkClientLib
{
public:
	CNetworkClientLib();
	~CNetworkClientLib();

	bool TryToConnectServer( const WCHAR* ip, USHORT port, int workerThreadCount, bool nagleOn);	//바인딩 IP, 서버IP / 워커스레드 수 / 나글옵션
	bool Disconnect();
	void SendMessages(CPacket*);

	void ReleaseSession();
	void RecvPost(SSession* session);
	void SendPost(SSession* session);

	virtual void OnEnterJoinServer() = 0;
	virtual void OnLeaveServer() = 0;
	virtual void OnMessage(CPacket*) = 0;
	virtual void OnError(int errorcode, const wchar_t* msg) = 0;
	virtual void OnThreadExit(const wchar_t* msg) = 0;


public:
	HANDLE GetExitEvent()
	{
		return hExitEvent;
	}
	HANDLE GetConnectEvent()
	{
		return hConnectEvent;
	}

	// thread 정보
	static unsigned int ConnectThread(LPVOID param);
	static unsigned int IOCPWorkerThread(LPVOID param);



private:
	HANDLE hExitEvent;
	HANDLE hConnectEvent;
	HANDLE hIOCP;
//	SOCKET sock;
	bool flag;

	SSession session;
	
	HANDLE hConnectThread;
	HANDLE hWorkerThread;
	HANDLE hSendThread;
	SOCKET serverSocket;
	wchar_t *serverIpAddress;
	USHORT serverPort;


	unsigned __int64 totalConnectCount;
	unsigned __int64 totalRecvCount;
	unsigned __int64 totalSendCount;
	unsigned __int64 connectTPS;
	unsigned __int64 recvTPS;
	unsigned __int64 sendTPS;
	unsigned __int64 totalSessionCount;
	unsigned __int64 sessionID;
};
