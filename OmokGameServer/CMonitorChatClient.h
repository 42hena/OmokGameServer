#pragma once


#define dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN 30		// 채팅서버 ChatServer 실행 여부 ON / OFF
#define dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU 31		// 채팅서버 ChatServer CPU 사용률
#define dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM 32		// 채팅서버 ChatServer 메모리 사용 MByte
#define dfMONITOR_DATA_TYPE_CHAT_SESSION 33		// 채팅서버 세션 수 (컨넥션 수)
#define dfMONITOR_DATA_TYPE_CHAT_PLAYER 34		// 채팅서버 인증성공 사용자 수 (실제 접속자)
#define dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS 35	// 채팅서버 UPDATE 스레드 초당 초리 횟수
#define dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL 36		// 채팅서버 패킷풀 사용량
#define dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL 37	// 채팅서버 UPDATE MSG 풀 사용량

#define en_PACKET_SS_MONITOR 20000
#define en_PACKET_SS_MONITOR_LOGIN 20001
#define en_PACKET_SS_MONITOR_DATA_UPDATE 20002

class CMonitorChatClient : public CNetworkClientLib
{
public:
	CMonitorChatClient(int serverNo);
	~CMonitorChatClient();

	virtual void OnEnterJoinServer();
	virtual void OnLeaveServer();
	virtual void OnMessage(CPacket*);
	virtual void OnError(int errorcode, const wchar_t* msg);
	virtual void OnThreadExit(const wchar_t* msg);

	// chatting
	void CreateMonitorLoginPacket(CPacket* packetPtr);
	void CreateDataUpdatePacket(CPacket* packetPtr, BYTE dataType);
	void SendChatInfo();


	HANDLE GetJobEvent();
	// Class 내의 스레드 정보.
	static unsigned int SendThread(LPVOID param);
	friend class CServer;


private:
	int		serverNo;
	HANDLE hJobEvent;

	long chatOnFlag;
	long chatCPUUsage;
	long chatMemUsage;
	long chatSessionCount;
	long chatPlayerCount;
	long chatUpdateTPS;
	long chatPacketPoolUsage;
	long chatJobPoolUsage;
};
