#include <unordered_map>
#include <Windows.h>

#include "CUser.h"
#include "CRoom.h"

void CChatRoom::PlaceStoneWrapper(int x, int y, int turn)
{
	// placeStone
	_omokBoard.PlaceStoneOnBoard(x, y, turn);
	
	// record
	Color col = static_cast<Color>(turn);
	_omokBoard.RecordPlaceStone(x, y, col);
}

int CChatRoom::CheckGameOverWrapper(int x, int y, int turn)
{
	// 0 ½ÂºÎ x | 1 1P | 2 2P | 3 draw
	if (_omokBoard.IsFullRecord())
		return 3;
	if (_omokBoard.CheckWinner(x, y, turn))
		return turn;
	return 0;
}

void CChatRoom::ResetGameData()
{
	_omokBoard.ResetOmok();
}
