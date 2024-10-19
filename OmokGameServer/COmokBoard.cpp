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
    // 8���� (������ �Ʒ� �밢��, �Ʒ�, ���� �Ʒ� �밢��, ������)
    const int dy[] = { 1, 1, 0, -1 };
    const int dx[] = { 1, 0, 1, 1 };

    // ���� ���� ���� (turn)
    int currentColor = _board[y][x];
    int maxBoardSize = static_cast<int>(Limit::MaxBoardSize);

    // �� ���⿡ ���� �˻�
    for (int dir = 0; dir < 4; ++dir)
    {
        int count = 1; // ���� ��ġ�� ���� �����ϹǷ� 1�� ����

        // ���� �������� ���� �� �� ���ӵǾ� �ִ��� Ȯ�� (���� ���)
        for (int i = 1; i < 5; ++i)
        {
            int ny = y + dy[dir] * i;
            int nx = x + dx[dir] * i;

            // ������ ����ų� �ٸ� ���� ���̸� ����
            if (ny < 0 || ny >= maxBoardSize || nx < 0 || nx >= maxBoardSize || _board[ny][nx] != currentColor)
                break;

            count++;
        }

        for (int i = 1; i < 5; ++i)
        {
            int ny = y - dy[dir] * i;
            int nx = x - dx[dir] * i;

            // ������ ����ų� �ٸ� ���� ���̸� ����
            if (ny < 0 || ny >= maxBoardSize || nx < 0 || nx >= maxBoardSize || _board[ny][nx] != currentColor)
                break;

            count++;
        }

        // 5�� �̻� �����̸� ���� ����
        if (count >= 5)
            return true;
    }

    // ��� ���⿡ ���� 5�� �̻� ���ӵ� ���� ���ٸ� ������ ������ ����
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