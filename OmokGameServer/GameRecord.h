#pragma once

enum class Color {	// ���� BYTE �غ����ϳ�.
	BLACK = 1,
	WHITE = 2,
};

struct BoardPos	
{
public:	// char�� �ٿ�����?
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
