#pragma once

#include <string>

//#include "Record.h"
#include "UserHistory.h"
#include "UserRoomInfo.h"


struct SLog
{
	WORD packetType;
	uintptr_t accountNo;
	WORD roomId;
	WORD state;
};

struct CUser
{
public:
	enum en_Position
	{
		NONE = 0,
		PLAYER1,
		PLAYER2,
		SPECTATOR
	};

public:
	CUser();
	~CUser() = default;

public:	// delete
	CUser(const CUser &rhs) = delete;
	CUser(CUser&& rhs) = delete;
	CUser& operator=(const CUser& rhs) = delete;
	CUser& operator=(CUser&& rhs) = delete;

public:
	inline uintptr_t GetCurrentAccountNo() const
	{
		return _accountNo;
	}
	inline int GetCurrentState() const
	{
		return static_cast<int>(_roomInfo._state);
		//return _state;
	}
	inline uintptr_t GetMySessionId() const
	{
		return _sessionId;
	}
	inline USHORT GetCurrentRoom() const
	{
		return _roomInfo.GetCurrentRoomNo();
		//return _roomInfo._inRoomNo;
		//return _inRoom;
	}
	inline const std::wstring& GetMyNickname() const
	{
		return _nickName;
	}

public:
	void RemovePosition()
	{
		_roomInfo.ClearPosition();
		//_state = NONE;
	}

	void ChangePositionPlayer1()
	{
		_roomInfo.ChangePositionToPlayerLeft();
		// _state = PLAYER1;
	}

	void ChangePositionPlayer2()
	{
		_roomInfo.ChangePositionToPlayerRight();
		//_state = PLAYER2;
	}

	void ChangePositionSpectator()
	{
		_roomInfo.ChangePositionToSpectator();
		// _state = SPECTATOR;
	}

	void EnterRoom(USHORT roomNo)
	{
		_roomInfo.EnterRoom(roomNo);
		//_inRoom = roomNo;
	}

	void LeaveRoom()
	{
		_roomInfo.LeaveRoom();
		//_inRoom = 0;
	}

	// ready 관련 고쳐야 함.
	void Ready()
	{
		_roomInfo.Ready();
		/*if (_readyFlag)
			DebugBreak();
		_readyFlag = true;*/
	}
	void CancelReady()
	{
		_roomInfo.CancelReady();
		/*if (!_readyFlag)
			DebugBreak();
		_readyFlag = false;*/
	}

	void ReadyClear()
	{
		_roomInfo.ReadyClear();
		//_readyFlag = false;
	}

	void Start()
	{
		Ready();
	}
	BYTE GetPosition()
	{
		return static_cast<BYTE>(_roomInfo.GetCurrentPosition());
	}
	void ChangeNickname(const std::wstring& changeNick);
	void GameClear();

	inline bool GetCurrentReadyFlag() const
	{
		return _roomInfo._readyFlag;
	}


	void Win();
	void Lost();
	void Draw();

	/*void Logging(WORD type, uintptr_t accountNo, WORD room, WORD state)
	{
		uintptr_t index = InterlockedIncrement(&logIndex);

		index %= 300;

		log[index].packetType = type;
		log[index].accountNo = accountNo;
		log[index].roomId = room;
		log[index].state = state;
	}*/

	

public:
	std::wstring _nickName;
	uintptr_t _accountNo;
	uintptr_t	_sessionId;
	uint32_t	_lastUpdateTime;

	//int _win;
	//int _lose;
	//bool _readyFlag;
	//int _inRoom;// 어느 방에 있는지
	//int _state;	// player인지, 관전자인지

	UserHistory _history;
	UserRoomInfo _roomInfo;

	//Inventory _inventory;
	//int _money;
	//SLog log[300];
	//alignas (64) uintptr_t logIndex = 0;
};

void WinProcedure(CUser*);
void LoseProcedure(CUser*);
void DrawProcedure(CUser*);
