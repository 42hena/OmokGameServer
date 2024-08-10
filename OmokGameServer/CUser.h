#pragma once

#include <string>


struct SPos
{
	int y;
	int x;
};

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
	CUser()
		: _nickName("unknown"),
		_accountNo(0),
		_inRoom(0),
		_state(0),
		_readyFlag(0),
		_sessionId(0),
		_lastUpdateTime(0),
		
		_win(0),
		_lose(0),
		_money(0)
		
	{

	}
	~CUser()
	{

	}

public:
	inline uintptr_t GetCurrentAccountNo() const
	{
		return _accountNo;
	}

	inline int GetCurrentState() const
	{
		return _state;
	}

	inline uintptr_t GetMySessionId() const
	{
		return _sessionId;
	}

	inline int GetCurrentRoom() const
	{
		return _inRoom;
	}

	inline const std::string& GetMyNickname() const
	{
		return _nickName;
	}

	void RemovePosition()
	{
		_state = NONE;
	}

	void ChangePositionPlayer1()
	{
		_state = PLAYER1;
	}

	void ChangePositionPlayer2()
	{
		_state = PLAYER2;
	}

	void ChangePositionSpectator()
	{
		_state = SPECTATOR;
	}

	void EnterRoom(USHORT roomId)
	{
		_inRoom = roomId;
	}

	void LeaveRoom()
	{
		_inRoom = 0;
	}

	// ready 관련
	void Ready()
	{
		if (_readyFlag)
			DebugBreak();
		_readyFlag = true;
	}
	void CancelReady()
	{
		if (!_readyFlag)
			DebugBreak();
		_readyFlag = false;
	}

	void ReadyClear()
	{
		_readyFlag = false;
	}

	void Start()
	{
		Ready();
	}

	void SetDBData(int win, int lose, int money)
	{
		if (_win < 0 || _lose < 0 || _money < 0)
			DebugBreak();

		_win = win;
		_lose = lose;
		_money = money;
	}

	void ChangeNickname(const std::string& changeNick)
	{
		_nickName = changeNick;
	}

	// 보류
	void GameClear()
	{
		_readyFlag = 0;
	}

	inline bool GetCurrentReadyFlag() const
	{
		return _readyFlag;
	}


	void Win()
	{
		_win++;
		_money += 150;
	}
	void Lost()
	{
		_lose++;
		_money += 50;
	}

	void Logging(WORD type, uintptr_t accountNo, WORD room, WORD state)
	{
		uintptr_t index = InterlockedIncrement(&logIndex);

		index %= 300;

		log[index].packetType = type;
		log[index].accountNo = accountNo;
		log[index].roomId = room;
		log[index].state = state;
	}

public:
	std::string _nickName;
	uintptr_t _accountNo;
	int _inRoom;// 어느 방에 있는지
	int _state;	// player인지, 관전자인지
	bool _readyFlag;
	uintptr_t	_sessionId;
	uint32_t	_lastUpdateTime;

	int _win;
	int _lose;
	int _money;

	SLog log[300];
	alignas (64) uintptr_t logIndex = 0;
};
