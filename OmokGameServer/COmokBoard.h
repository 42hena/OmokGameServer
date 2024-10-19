#pragma once

#include "GameRecord.h"

class COmokBoard
{
public:
	enum class Limit {
		MaxBoardSize = 15,
	};

public:
    COmokBoard();
    ~COmokBoard();

    // board[][]
    bool IsValidPos(int x, int y) const;
    bool CheckWinner(int x, int y, int turn)const;
    void PlaceStoneOnBoard(int x, int y, int turn);
    void ClearBoard();

    // omokRecord Part
    inline int GetStoneCount() const { return _omokRecord._stoneCount; }
    inline void ResetRecordData() { _omokRecord._stoneCount = 0; }
    inline bool IsFullRecord() const { return _omokRecord._stoneCount == static_cast<int>(GameRecord::Limit::MaxStoneCount) ? true : false; }
    inline BoardPos GetGameProgress(int idx) { return _omokRecord._gameRecord[idx];  }
    void RecordPlaceStone(int x, int y, Color turn);
    inline void RecodeUp() { _omokRecord._stoneCount++; }
    inline bool CanGetPos(int idx)  const {return idx < GetStoneCount() ? true : false; }
    inline BoardPos GetPos(int idx) const { return _omokRecord._gameRecord[idx]; }
    
    // gameFlag    
    void GameStart();
    inline bool IsGameing() const { return _gameFlag; }
    inline void ChangeModeToGameStart() { _gameFlag = 1; }
    inline void ChangeModeToGameOver() { _gameFlag = 0; }

    // _turn
    inline void InitTurn() { _turn = 0; }
    inline bool GetCurrentPlayerTurn() const { return _turn; }
    //inline void SwitchNext() { _turn++; }
    inline void SwitchNext() { _turn = !_turn; }

    // Util
    void ResetOmok();
private:
	int _board [static_cast<int>(Limit::MaxBoardSize)][static_cast<int>(Limit::MaxBoardSize)];
	GameRecord _omokRecord;
    bool _gameFlag;
    bool _turn;
};
