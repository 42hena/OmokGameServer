#pragma once
#include <vector>
class Player
{

};

struct Pos
{
	int x;
	int y;
};

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
	IRoom& operator=(const IRoom& rhs) = delete;

public:
	virtual void EnterRoom(uintptr_t accountNo, CUser* pNewUser) = 0;
	virtual void LeaveRoom(uintptr_t accountNo, CUser* pOldUser) = 0;

	int GetCurrentRoomNumber() const
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
		_gameFlag(0),
		_turn(0)
		//_stoneRecord
	{
		for (int i = 0; i < 15; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				_board[i][j] = 0;
			}
		}
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

	// 게임 쪽 생각해보기.
	void BoardClear()
	{
		for (int i = 0; i < 15; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				_board[i][j] = 0;
			}
		}
		

		// DB 저장 해야함.
		// 승 패 누가 했는지
		// 아이템 누가 몇개 뭐 썼는지
		// dak.gg처럼 어떻게 두었는지.
	}

	bool PossibleStone(int x, int y)
	{
		if (x < 0 || x >= 15)
			return false;
		if (y < 0 || y >= 15)
			return false;
		return _board[x][y] ? false : true;
	}

	bool IsGameOver(int x, int y)
	{
		const int dx[] = { 1, 1, 1, 0, -1, -1, -1, 0 };
		const int dy[] = { 1, 0, -1, 1, 1, 0, -1, -1 };
		//for (int i = 0 ; i < )
	}
	
	/*void ProcessingDB()
	{

	}*/


	inline bool IsGameing() const
	{
		return _gameFlag;
	}

	void GameStart()
	{
		if (_gameFlag)
			DebugBreak();
		_gameFlag = true;
	}

	void GameEnd()
	{
		if (!_gameFlag)
			DebugBreak();
		_gameFlag = false;
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

	void ProcessRecord(int x, int y)
	{
		_stoneRecord.push_back({ x, y });
	}

	void ReDoRecord(int x, int y)
	{
		_stoneRecord.pop_back();
	}

	void PutStone(int x, int y, int position)
	{
		_board[x][y] = position;
	}
	void CancelStone(int x, int y)
	{
		_board[x][y] = 0;
	}


	bool GameEnd(int x, int y, int player)
	{
		const int directions[4][2] = {
		   {1, 0}, // horizontal
		   {0, 1}, // vertical
		   {1, 1}, // diagonal1 (top-left to bottom-right)
		   {1, -1} // diagonal2 (bottom-left to top-right)
		};
		const int WinCondition = 5;
		const int BoardSize = 15;

		for (int dir = 0; dir < 4; ++dir) {
			int count = 1;

			// Check in the positive direction
			for (int step = 1; step < WinCondition; ++step) {
				int nx = x + step * directions[dir][0];
				int ny = y + step * directions[dir][1];

				if (nx >= 0 && nx < BoardSize && ny >= 0 && ny < BoardSize && _board[nx][ny] == player) {
					count++;
				}
				else {
					break;
				}
			}

			// Check in the negative direction
			for (int step = 1; step < 5; ++step) {
				int nx = x - step * directions[dir][0];
				int ny = y - step * directions[dir][1];

				if (nx >= 0 && nx < BoardSize && ny >= 0 && ny < BoardSize && _board[nx][ny] == player) {
					count++;
				}
				else {
					break;
				}
			}

			// Check if we have enough stones in a row
			if (count >= 5) {
				return true;
			}
		}

		return false;
	}

	/*CUser* GetOppUser(int position)
	{
		if (position == 1)
		{

		}
		else if (position == 2)
		{

		}
		else
			DebugBreak();

	}*/

	inline BYTE GetCurrentTurn() const
	{
		return _turn;
	}
	void InitTurn()
	{
		_turn = 0;
	}

	void NextTurn()
	{
		if (_turn == 0)
			_turn = 1;
		else if (_turn == 1)
			_turn = 2;
		else if (_turn == 2)
			_turn = 1;
	}

private:
	bool _gameFlag = 0;
	SPlayerData _players[2];
	std::vector<Pos> _stoneRecord;
	int _board[15][15] = { 0 };
	BYTE _turn;
};

//int CChatRoom::num = 1;
//CChatRoom::CChatRoom(uintptr_t roomNumber)
//	: IRoom(roomNumber)
//{
//}
//
//CChatRoom::~CChatRoom()
//{
//}

// Contents는 CRoom들을 들고 있을 거임.
// player 들은 자신 소속을 알고 있음. empty(Lobby) 특정 방.
// 만약 방에 들어가고 싶다?
// 1. RoomManager를 확인하고 있을 경우 지정 -> 들어가기.
// 2. 없다면 방을 만들고 들어가기.
// 

