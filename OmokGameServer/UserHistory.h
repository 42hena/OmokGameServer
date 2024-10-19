#pragma once

struct UserHistory
{
	UserHistory();
	~UserHistory();

	UserHistory(const UserHistory& rhs) = delete;
	UserHistory(UserHistory&& rhs) = delete;
	UserHistory& operator=(const UserHistory& rhs) = delete;
	UserHistory& operator=(UserHistory&& rhs) = delete;

	inline int GetWinCount()
	{
		return _win;
	}
	inline int GetDrawCount()
	{
		return _draw;
	}
	inline int GetLoseCount()
	{
		return _lose;
	}
	inline int GetRating()
	{
		return _rating;
	}

	void InitRecord();
	double GetWinRate();
	void Win();
	void Draw();
	void Lose();

public:
	int _win;
	int _lose;
	int _draw;
	int _rating;
};

void NoramlGameOver(UserHistory&, UserHistory&);
void DrawGameOver(UserHistory&, UserHistory&);
void abnormalGameOver(UserHistory&, UserHistory&);
