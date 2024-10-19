#include "UserHistory.h"

UserHistory::UserHistory()
	:_win(0),
	_draw(0),
	_lose(0),
	_rating(1500)
{ }

UserHistory::~UserHistory()
{ }

void UserHistory::Win()
{
	_win += 1;
}

void UserHistory::Draw()
{
	_draw += 1;
}

void UserHistory::Lose()
{
	_lose += 1;
}

double UserHistory::GetWinRate()
{
	return static_cast<double>(_win) / (_win + _draw + _lose);
}

void UserHistory::InitRecord()
{
	_win = 0;
	_draw = 0;
	_lose = 0;
}


// Record Util
void NoramlGameOver(UserHistory& winner, UserHistory& loser)
{
	winner.Win();
	loser.Lose();
}

void DrawGameOver(UserHistory& player1, UserHistory& player2)
{
	player1.Draw();
	player2.Draw();
}

void abnormalGameOver(UserHistory& winner, UserHistory& loser)
{
	NoramlGameOver(winner, loser);
}