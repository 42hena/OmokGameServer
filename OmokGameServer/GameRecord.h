#pragma once

enum class Color {	// 형식 BYTE 해봐야하나.
	BLACK = 1,
	WHITE = 2,
};

struct BoardPos	
{
public:	// char로 줄여볼까?
	int _y;
	int _x;
	Color _color;
};

struct GameRecord
{
public:
	enum class Limit {
		MaxStoneCount = 255,
	};

public:
	BoardPos _gameRecord[255];
	int _stoneCount;
};
