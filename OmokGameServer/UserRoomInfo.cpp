#include <Windows.h>

#include "UserRoomInfo.h"

UserRoomInfo::UserRoomInfo()
	: _inRoomNo(0),
	_state(Position::None),
	_readyFlag(0)
{ }

UserRoomInfo::Position UserRoomInfo::GetCurrentPosition()
{
	return _state;
}
void UserRoomInfo::ChangePositionToPlayerLeft()
{
	_state = Position::Player1;
}

void UserRoomInfo::ChangePositionToPlayerRight()
{
	_state = Position::Player2;
}

void UserRoomInfo::ChangePositionToSpectator()
{
	_state = Position::Spectator;
}

void UserRoomInfo::ClearPosition()
{
	_state = Position::None;
}

void UserRoomInfo::EnterRoom(USHORT roomNo)
{
	_inRoomNo = roomNo;
}

void UserRoomInfo::LeaveRoom()
{
	_inRoomNo = 0;
}

void UserRoomInfo::Ready()
{
	if (_readyFlag)
		DebugBreak();
	_readyFlag = true;
}
void UserRoomInfo::CancelReady()
{
	if (!_readyFlag)
		DebugBreak();
	_readyFlag = false;
}

void UserRoomInfo::ReadyClear()
{
	_readyFlag = false;
}
