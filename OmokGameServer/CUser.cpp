#include <string>
#include <Windows.h>
#include "CUser.h"

CUser::CUser()
	: _nickName(L"unknown"),
	_accountNo(0),
	_sessionId(0),
	_lastUpdateTime(0),

	//_readyFlag(0),
	//_inRoom(0),
	//_state(0),
	//_win(0),
	//_lose(0),

	_history(),
	_roomInfo()
{ }

void CUser::Win()
{
	_history.Win();
	_roomInfo.ReadyClear();
}

void CUser::Lost()
{
	_history.Lose();
	_roomInfo.ReadyClear();
}

void CUser::Draw()
{
	_history.Draw();
	_roomInfo.ReadyClear();
}

void CUser::ChangeNickname(const std::wstring& changeNick)
{
	_nickName = changeNick;
}

void CUser::GameClear()
{
	_roomInfo.ReadyClear();
}

void WinProcedure(CUser* pUser)
{
	pUser->Win();
}

void LoseProcedure(CUser* pUser)
{
	pUser->Lost();
}

void DrawProcedure(CUser* pUser)
{
	pUser->Draw();
}
