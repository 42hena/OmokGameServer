#pragma once

class CServer : public CNetLibrary
{
public:
	CServer();
	virtual ~CServer();

	// 사용하지 않는 함수들
	CServer(const CServer& copy) = delete;
	CServer(CServer&& copy) = delete;
	CServer& operator=(const CServer& copy) = delete;
	CServer& operator=(CServer&& copy) = delete;

public:
	virtual bool OnConnectionRequest(const WCHAR* IPAddress, USHORT port) override;
	virtual void OnClientJoin(unsigned __int64 sessionID) override;
	virtual void OnClientLeave(unsigned __int64 SessionID) override;
	virtual void OnMessage(unsigned __int64 sessionID, CPacket* packet) override;
	virtual void OnError(int errorcode, const WCHAR* msg) override;

public:
	static unsigned int __stdcall UpdateThread(LPVOID param);
	static unsigned int __stdcall MonitorThread(LPVOID param);
	
public:
	void CreateUser(unsigned __int64 id);
	CUser* FindUser(unsigned __int64 sID);
	CChatRoom* FindRoom(WORD roomNo);
	CChatRoom* FindRoomTmp(WORD roomNo);

	// Default
	void MakeResponseLoginPacket(CUser*, CPacket*);
	void MakeResponseGracefulShutdownPacket(CUser*, CPacket*);
	void MakeCreateRoomPacket(CUser*, CPacket*, WORD);
	void MakeResponseEnterRoomPacket(CUser*, CChatRoom*, CPacket*, BYTE);
	void MakeEnterAlarmPacket(CUser*, CChatRoom*, CPacket*);
	void MakeRoomMembersPacket(CUser*r, CChatRoom*, CPacket*);
	void MakeResponseLeaveRoomPacket(CUser*, CChatRoom*, CPacket*, BYTE);
	void MakeLeaveAlarmPacket(CUser*, CChatRoom*, CPacket*);	// 403

	void MakeResponseGetRoomList(USHORT, CUser* , CPacket*); // 애매...
	void MakeEnterPlayerAlarmPacket(CChatRoom* pRoom, CPacket* pPacket);

	// Today
	void MakeResponseChangePositionPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE, BYTE, BYTE);
	void MakeResponseReadyPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket);
	void MakeResponseCancelPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE);
	void MakeResponsePutStonePacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket, BYTE flag, BYTE x, BYTE y);
	
	
	void MakeGameStartPacket(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket);
	void MakeGameOverPacket(CUser* pUser, CUser*, CChatRoom* pRoom, CPacket* pPacket, int flag);
	void MakeRecordPacket(CChatRoom* pRoom, CPacket* pPacket);



	void MakeUserAlarm(CUser*pUser, CChatRoom*pRoom, CPacket*pPacket)
	{
		const auto packetType = static_cast<WORD>(1234);
		
		// type 지정 필요.
		if (pUser == nullptr || pRoom == nullptr)
		{
			DebugBreak();
			return;
		}
		pPacket->Clear();

		BYTE userCnt = 0;
		BYTE nickLen = 0;
		BYTE pos;
		BYTE readyFlag;
		*pPacket << packetType << userCnt;
		uintptr_t accountNo = pRoom->GetPlayer1AccountNo();
		if (accountNo > 0)
		{
			auto p1 = pRoom->FindUser(accountNo);
			std::wstring nick = p1->GetMyNickname();
			nickLen = nick.size();
			pos = 1;
			readyFlag = p1->GetCurrentReadyFlag();

			*pPacket << pos << readyFlag << nickLen;
			pPacket->PutData((char*)nick.c_str(), nickLen * 2);
		}
		accountNo = pRoom->GetPlayer2AccountNo();
		if (accountNo > 0)
		{
			auto p2 = pRoom->FindUser(accountNo);
			std::wstring nick = p2->GetMyNickname();
			nickLen = nick.size();
			pos = 2;
			readyFlag = p2->GetCurrentReadyFlag();

			*pPacket << pos << readyFlag << nickLen;
			pPacket->PutData((char*)nick.c_str(), nickLen * 2);
		}
	}
	
	// 
	//void MakeRecordPacket(CChatRoom* pRoom, CPacket* pPacket)
	//{
	//	const auto packetType = static_cast<WORD>(1234);
	//	if (pRoom == nullptr)
	//	{
	//		DebugBreak();
	//		return;
	//	}

	//	//BYTE cnt = pRoom-> 카운트
	//	pPacket->Clear();
	//	*pPacket << packetType;
	//}
	
	// Game

	void GameStartProcedure(CUser* pUser, CChatRoom* pRoom, CPacket* pPacket);

	// Item
	void MakeResponseGetItemList();
	void MakeResponsePurchaseItem();

public:
	void EnterSuccess(uintptr_t, CUser*, CChatRoom*);
	void LeaveSuccess(uintptr_t, CUser*, CChatRoom*);
	void ChangeSuccess(uintptr_t, CUser*, CChatRoom*, BYTE);


public:
public:
	CPacket* MakeCreateRoomPacket(CUser*, WORD);
	CPacket* MakeGracefulShutdownPacket(CUser*);
	CPacket* MakeEnterRoomPacket(CUser* user, CChatRoom* room, BYTE status);
	CPacket* MakeEnterRoomAlarmPacket(CUser* user, CChatRoom* room);
	CPacket* MakeGetUserListPacket(CUser* user, CChatRoom* room);
	
	
	CPacket* MakeLeaveRoomPacket(CUser* user, CChatRoom* room, BYTE status);
	void MakeGetUserListPacket(CUser* user, CChatRoom* room, CPacket* packet);	// 304


	void MakeChatPacket(CUser* user, CChatRoom* room, CPacket* packet, const std::wstring& chat);

	void SendResponseMessage(uintptr_t, CPacket*);
	void SendResponseMessageAll(int roomNum, CPacket* pPacket, UINT64 accountNo);


	// OnMessage
	void Echo(unsigned __int64 sID, CPacket* packet);
	void Recv(unsigned __int64 sID, CPacket* packet);
	void SendAround(int sx, int sy, CPacket* packet);
	void GracefulShutdown(unsigned __int64 id, CPacket* pPacket);

	void RoomList(unsigned __int64 sID, CPacket* pRecvPacket);
	void EnterRoom(unsigned __int64 sID, CPacket* packet);

	// procedure
	void LoginProcedure(unsigned __int64, CPacket*);
	void CreateRoomProcedure(unsigned __int64, CPacket*);
	void EnterRoomProcedure(unsigned __int64, CPacket*);
	void LeaveRoomProcedure(unsigned __int64, CPacket*);
	void ChattingProcedure(unsigned __int64, CPacket*);

	void GameStartProcedure(unsigned __int64, CPacket*);
	void GameOverProcedure(CUser* pUser, CChatRoom* pRoom, int endFlag);




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

	void SendRoomAll(USHORT roomNum, CPacket* packet);
	void SendRoomAll(USHORT roomNum, CPacket* packet, UINT64 accountNo);
	//void FindAccountNo

	inline size_t GetUserCount()
	{
		return _accountKey.size();
	}

	void InsertKey(uintptr_t accountNo, uintptr_t id)
	{
		_accountKey.insert({ accountNo, id });
	}
	void DeleteKey(uintptr_t accountNo)
	{
		auto eraseSize = _accountKey.erase(accountNo);
		if (eraseSize == 0)
			DebugBreak();
	}

	void LobbyUp()
	{
		InterlockedIncrement(&_lobbyPeopleCount);
	}
	void LobbyDown()
	{
		InterlockedDecrement(&_lobbyPeopleCount);
	}
	void RoomUp()
	{
		InterlockedIncrement(&_roomPeopleCount);
	}
	void RoomDown()
	{
		InterlockedDecrement(&_roomPeopleCount);
	}

	void CreateCountUp()
	{
		InterlockedIncrement(&_createRoomPacketCount);
	}

	void EnterCountUp()
	{
		InterlockedIncrement(&_enterRoomPacketCount);
	}
	void LeaveCountUp()
	{
		InterlockedIncrement(&_leaveRoomPacketCount);
	}
	void ChatCountUp()
	{
		InterlockedIncrement(&_chatRoomPacketCount);
	}

	inline long GetLobbyUserCount()
	{
		return _lobbyPeopleCount;
	}
	inline long GetRoomUserCount()
	{
		return _roomPeopleCount;
	}

	inline long GetAndInitCreateRoomPacketCount()
	{
		return InterlockedExchange(&_createRoomPacketCount, 0);
	}
	inline long GetAndInitEnterRoomPacketCount()
	{
		return InterlockedExchange(&_enterRoomPacketCount, 0);
	}
	inline long GetAndInitLeaveRoomPacketCount()
	{
		return InterlockedExchange(&_leaveRoomPacketCount, 0);
	}
	inline long GetAndInitChatRoomPacketCount()
	{
		return InterlockedExchange(&_chatRoomPacketCount, 0);
	}

private:
	alignas (64) long userCount;
	alignas (64) long updateCount;
	alignas (64) long _lobbyPeopleCount;
	alignas (64) long _roomPeopleCount;
	long _createRoomPacketCount;
	long _enterRoomPacketCount;
	long _leaveRoomPacketCount;
	long _chatRoomPacketCount;
	CMemoryPool<CUser> userPool {16384, true};

	std::unordered_map<uint64_t, CChatRoom*> _roomManager;
	CMemoryPool<CChatRoom> _roomPool {500, false};

private:
	std::unordered_map<uintptr_t, uintptr_t> _accountKey;

	HANDLE hUpdateTh;
	HANDLE hMonitorTh;
	
};


//void ChangeDefaultStatus(CUser* pUser, CChatRoom* pRoom)
//{
//	//pUser->_inRoom 
//	pUser->ChangePositionSpectator();
//	pRoom->AddUser( pUser->GetCurrentAccountNo(), pUser);
//}


extern CLFQueue<SJob> jobQ;
extern uintptr_t messageRecvCount;
extern uintptr_t messageSendCount;
extern uintptr_t loginPacket;
extern uintptr_t messagePacket;
