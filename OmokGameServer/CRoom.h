#pragma once
#include <vector>
class Player
{

};

#include "COmokBoard.h"
class IRoom
{
public:
	//IRoom()
	IRoom()
		: _roomNum(0)
	{ }
	IRoom(uintptr_t roomNumber)
		: _roomNum(roomNumber)
	{ }
	virtual ~IRoom()
	{
		_userMap.clear();
	}

	IRoom(const IRoom& rhs) = delete;
	IRoom(IRoom&& rhs) = delete;
	IRoom& operator=(const IRoom& rhs) = delete;
	IRoom& operator=(IRoom&& rhs) = delete;

public:
	virtual void EnterRoom(uintptr_t accountNo, CUser* pNewUser) = 0;
	virtual void LeaveRoom(uintptr_t accountNo, CUser* pOldUser) = 0;

	USHORT GetCurrentRoomNumber() const
	{
		return _roomNum;
	}
	void ChangeRoomNumber(int number)
	{
		_roomNum = number;
	}

	const std::string& GetCurrentRoomName() const
	{
		return _roomName;
	}
	void ChangeRoomName(const std::string& name)
	{
		_roomName = name;
	}

	bool CheckUser(uint64_t accountNo)
	{
		auto it = _userMap.find(accountNo);

		if (it == _userMap.end())
			return false;
		return true;
	}

	void AddUser(uint64_t accountNo, CUser * newUser)
	{
		_userMap.insert({ accountNo, newUser });
	}

	void EraseUser(uint64_t accountNo)
	{
		auto it = _userMap.find(accountNo);
		_userMap.erase(it);
	}

	CUser* FindUser(uint64_t accountNo)
	{
		auto it = _userMap.find(accountNo);
		if (it == _userMap.end())
			return nullptr;
		else
			return it->second;
	}
	USHORT GetRoomUserCount()
	{
		return _userMap.size();
	}

protected:
	std::unordered_map<uint64_t, CUser*> _userMap;

private:
	int _roomNum;
	std::string _roomName;
};

struct SPlayerData
{
	SPlayerData()
		: _sId(0),
		_accountNo(0)
	{}
	uintptr_t _sId;
	uintptr_t _accountNo;
};

class CChatRoom : public IRoom
{
public:
	//CChatRoom();
	CChatRoom()
		:IRoom(0),
		_playerCnt(0)
		//_stoneRecord
	{
	}
	~CChatRoom()
	{
	}

	CChatRoom(const CChatRoom& rhs) = delete;
	CChatRoom& operator=(const CChatRoom& rhs) = delete;

public:
	virtual void EnterRoom(uintptr_t accountNo, CUser* pNewUser)	// Base pure
	{
		AddUser(accountNo, pNewUser);
	}
	virtual void LeaveRoom(uintptr_t accountNo, CUser* pOldUser)	// Base pure
	{
		EraseUser(accountNo);
	}

	std::unordered_map<uint64_t, CUser*> GetUserList()
	{
		return _userMap;
	}

	int GetUserCount()
	{
		return _userMap.size();
	}

	void SendChatting()
	{
		for (auto it = _userMap.begin(); it != _userMap.end(); ++it)
		{
			
		}
	}


	bool PossibleStone(int x, int y)
	{
		return _omokBoard.IsValidPos(x, y);
	}

	inline bool IsGameing() const
	{
		return _omokBoard.IsGameing();
	}
	
	inline uintptr_t GetPlayer1SessionId() const
	{
		return _players[0]._sId;
	}
	inline uintptr_t GetPlayer2SessionId() const
	{
		return _players[1]._sId;
	}
	inline uintptr_t GetPlayer1AccountNo() const
	{
		return _players[0]._accountNo;
	}
	inline uintptr_t GetPlayer2AccountNo() const
	{
		return _players[1]._accountNo;
	}

	bool IsPossibleChangePositionPlayer(BYTE position)
	{
		if (position == 1)
		{
			if (_players[0]._accountNo > 0)
				return false;
			return true;
		}
		else if (position == 2)
		{
			if (_players[1]._accountNo > 0)
				return false;
			return true;
		}
		else if (position == 3)
			return true;
		else
			DebugBreak();
	}

	void Init()
	{
		_players[0]._accountNo = 0;
		_players[1]._accountNo = 0;
		_players[0]._sId = 0;
		_players[1]._sId = 0;
	}

	void Player1Clear()
	{
		_players[0]._accountNo = 0;
		_players[0]._sId = 0;
	}

	void Player2Clear()
	{
		_players[1]._accountNo = 0;
		_players[1]._sId = 0;
	}

	void SetPlayer1(uintptr_t accountNo, uintptr_t sId)
	{
		_players[0]._accountNo = accountNo;
		_players[0]._sId = sId;
	}

	void SetPlayer2(uintptr_t accountNo, uintptr_t sId)
	{
		_players[1]._accountNo = accountNo;
		_players[1]._sId = sId;
	}

	void ReadyPlayer(int position, uintptr_t accountNo, uintptr_t sId)
	{
		if (position == 1)
		{
			SetPlayer1(accountNo, sId);
		}
		else if (position == 2)
		{
			SetPlayer2(accountNo, sId);
		}
		else
		{
			DebugBreak();
		}
	}

	void CancelPlayer(int position, uintptr_t accountNo, uintptr_t sId)
	{
		if (position == 1 || position == 2)
		{
			if (_players[position - 1]._accountNo != accountNo)
			{
				DebugBreak();
			}
			if (_players[position - 1]._sId != sId)
			{
				DebugBreak();
			}

			_players[position - 1]._accountNo = 0;
			_players[position - 1]._sId = 0;
		}
		else
		{
			return;
		}
	}

	inline BYTE GetCurrentTurn()
	{
		return _omokBoard.GetCurrentPlayerTurn();
	}

	void pCountUp()
	{
		_playerCnt++;
	}
	void pCountDown()
	{
		_playerCnt--;
	}
	int GetPlayerCount()
	{
		return _playerCnt;
	}

	COmokBoard& GetBoard()
	{
		return _omokBoard;
	}

	void TurnEnd()
	{
		_omokBoard.SwitchNext();
	}

	// Omok Game Part
	void PlaceStoneWrapper(int x, int y, int turn);
	int CheckGameOverWrapper(int x, int y, int turn);
	void ResetGameData();


	void InitGameSetting()
	{
		_omokBoard.ResetOmok();
		_omokBoard.GameStart();
	}
private:
	//bool _gameFlag = 0;
	SPlayerData _players[2];
	int _playerCnt;
	//BYTE _turn;
	COmokBoard _omokBoard;
};

