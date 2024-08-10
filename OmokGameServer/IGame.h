#pragma once


class IGame
{
public:
	IGame();
	~IGame();

private:

};

IGame::IGame()
{
}

IGame::~IGame()
{
}

class Player;

class COmok : public IGame
{
public:


private:
	Player* player[2];
};