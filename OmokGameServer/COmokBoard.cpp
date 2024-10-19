#include "COmokBoard.h"

COmokBoard::COmokBoard()
	:_omokRecord(),
	_gameFlag(0)
{

	ClearBoard();
}

COmokBoard::~COmokBoard()
{}

bool COmokBoard::IsValidPos(int x, int y)const
{
	int maxBoardSize = static_cast<int>(Limit::MaxBoardSize);
	if (y < 0 || y >= maxBoardSize || x < 0 || x >= maxBoardSize)
		return false;
	if (_board[y][x])
		return false;
	return true;
}

bool COmokBoard::CheckWinner(int x, int y, int turn)const
{
    // 8방향 (오른쪽 아래 대각선, 아래, 왼쪽 아래 대각선, 오른쪽)
    const int dy[] = { 1, 1, 0, -1 };
    const int dx[] = { 1, 0, 1, 1 };

    // 현재 돌의 색상 (turn)
    int currentColor = _board[y][x];
    int maxBoardSize = static_cast<int>(Limit::MaxBoardSize);

    // 각 방향에 대해 검사
    for (int dir = 0; dir < 4; ++dir)
    {
        int count = 1; // 현재 위치의 돌도 포함하므로 1로 시작

        // 현재 방향으로 돌이 몇 개 연속되어 있는지 확인 (양쪽 모두)
        for (int i = 1; i < 5; ++i)
        {
            int ny = y + dy[dir] * i;
            int nx = x + dx[dir] * i;

            // 범위를 벗어나거나 다른 색의 돌이면 중지
            if (ny < 0 || ny >= maxBoardSize || nx < 0 || nx >= maxBoardSize || _board[ny][nx] != currentColor)
                break;

            count++;
        }

        for (int i = 1; i < 5; ++i)
        {
            int ny = y - dy[dir] * i;
            int nx = x - dx[dir] * i;

            // 범위를 벗어나거나 다른 색의 돌이면 중지
            if (ny < 0 || ny >= maxBoardSize || nx < 0 || nx >= maxBoardSize || _board[ny][nx] != currentColor)
                break;

            count++;
        }

        // 5개 이상 연속이면 게임 종료
        if (count >= 5)
            return true;
    }

    // 모든 방향에 대해 5개 이상 연속된 돌이 없다면 게임은 끝나지 않음
    return false;
}


void COmokBoard::PlaceStoneOnBoard(int x, int y, int turn)
{
    _board[y][x] = turn;
}


void COmokBoard::RecordPlaceStone(int x, int y, Color turn)
{
    _omokRecord._gameRecord[GetStoneCount()]._x = x;
    _omokRecord._gameRecord[GetStoneCount()]._y = y;
    _omokRecord._gameRecord[GetStoneCount()]._color = turn;
    RecodeUp();
}
void COmokBoard::ClearBoard()
{
    int maxBoardSize = static_cast<int>(Limit::MaxBoardSize);

    for (int i = 0; i < maxBoardSize; ++i)
    {
        for (int j = 0; j < maxBoardSize; ++j)
        {
            _board[i][j] = 0;
        }
    }
}

void COmokBoard::ResetOmok()
{
    //_turn
    InitTurn();

    // board[][] 
    ClearBoard();

    // gameFlag
    ChangeModeToGameOver();

    // _omokRecord
    ResetRecordData();
}

void COmokBoard::GameStart()
{
    //_turn
    InitTurn();

    // gameFlag
    ChangeModeToGameStart();
}