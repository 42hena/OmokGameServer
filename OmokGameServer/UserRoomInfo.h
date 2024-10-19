#pragma once

struct UserRoomInfo
{
public:
	enum class Position {
		None,
		Player1,
		Player2,
		Spectator,
	};

public:
	UserRoomInfo();
	~UserRoomInfo() = default;

public:
	UserRoomInfo(const UserRoomInfo& rhs) = delete;
	UserRoomInfo(UserRoomInfo&& rhs) = delete;
	UserRoomInfo& operator=(const UserRoomInfo& rhs) = delete;
	UserRoomInfo& operator=(UserRoomInfo&& rhs) = delete;

	inline USHORT GetCurrentRoomNo() const
	{
		return _inRoomNo;
	}
	
	Position GetCurrentPosition();

	void ChangePositionToPlayerLeft();
	void ChangePositionToPlayerRight();
	void ChangePositionToSpectator();
	void ClearPosition();
	void EnterRoom(USHORT);
	void LeaveRoom();
	void Ready();
	void CancelReady();
	void ReadyClear();

public:
	int _inRoomNo;// 어느 방에 있는지
	Position _state;	// player인지, 관전자인지
	bool _readyFlag;
};