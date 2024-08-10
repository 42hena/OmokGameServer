#pragma once
#ifndef __CLASS_SERVER_H__
#define __CLASS_SERVER_H__

class CServer : public CNetLibrary
{
public:
	// ##################################################
	// #                기본     함수들                 #
	// ##################################################
	CServer();
	virtual ~CServer();

	// 사용하지 않는 함수들
	CServer(const CServer& copy) = delete;				// c++98이면 private로 바꾸기
	CServer& operator=(const CServer& copy) = delete;	// c++98이면 private로 바꾸기

	// ##################################################
	// #                외부 노출 함수                  #
	// ##################################################
	virtual bool OnConnectionRequest(const WCHAR* IPAddress, USHORT port);
	virtual void OnClientJoin(unsigned __int64 sessionID);
	virtual void OnClientLeave(unsigned __int64 SessionID);
	virtual void OnMessage(unsigned __int64 sessionID, CPacket* packet);
	virtual void OnError(int errorcode, const WCHAR* msg);

	static unsigned int __stdcall UpdateThread(LPVOID param);
	//static unsigned int __stdcall MonitorThread(LPVOID param);
	
	void CreateUser(unsigned __int64 id);
	//void CreateUser(uintptr_t id);
	CUser* FindUser(unsigned __int64 sID);

	// OnMessage
	int LoginTest(unsigned __int64 sID, CPacket* packet);
	void Recv(unsigned __int64 sID, CPacket* packet);
	void SendAround(int sx, int sy, CPacket* packet);


	void CreateNewRoom(unsigned __int64 sID, CPacket* packet);
	void RoomList(unsigned __int64 sID, CPacket* pRecvPacket);
	void EnterRoom(unsigned __int64 sID, CPacket* packet);


	void EnterRoomResponse(unsigned __int64 sID, CPacket* packet);
	void LeaveRoomResponse(unsigned __int64 sID, CPacket* pRecvPacket);

	void ChatRoom(unsigned __int64 sID, CPacket* pRecvPacket);

	void ChangePosition(unsigned __int64 sID, CPacket* pRecvPacket);

	void Ready(unsigned __int64 sID, CPacket* pRecvPacket);
	void Start(unsigned __int64 sID, CPacket* pRecvPacket);

	void CancelReady(unsigned __int64 sID, CPacket* pRecvPacket);

	void PutStone(unsigned __int64 sId, CPacket* pPacket);

	void ReleaseUser(unsigned __int64 sID);
	/*long GetUser()
	{
		return userCount;
	}
	long GetMonitoringUpdateCount()
	{
		return InterlockedExchange(&updateCount, 0);
	}

	long GetUserPoolUseSize()
	{
		return userPool.GetUseCount();
	}
	long GetUserPoolAllocSize()
	{
		return userPool.GetAllocCount();
	}*/
	//void ConnectToMonitorServer();

	void SendRoomAll(int roomNum, CPacket* packet);
private:
	alignas (64) long userCount;
	alignas (64) long updateCount;
	CMemoryPool<CUser> userPool {16384, true};

	std::unordered_map<uint64_t, CChatRoom*> _roomManager;
	CMemoryPool<CChatRoom> roomPool {500, false};
};

extern CLFQueue<SJob> jobQ;
extern uintptr_t messageRecvCount;
extern uintptr_t messageSendCount;
extern uintptr_t loginPacket;
extern uintptr_t messagePacket;
extern uintptr_t sectorPacket;
#endif