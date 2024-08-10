#pragma once

class CGroupManager {
	friend class CNetLibrary;
public:
	//CGroupManager();
	CGroupManager(void *ptr);
	~CGroupManager();

public:

	CNetLibrary* netLib;

	CLobby lobby;
	CField field;

	// Accept -> Lobby
	void MoveDefault(unsigned __int64 sessionId, SSession* sessionPtr);

	// Lobby -> Field
	void AuthSuccess(unsigned __int64 sessionId, SSession* sessionPtr);
};