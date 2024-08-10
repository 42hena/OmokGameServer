#pragma once

#include <stack>

class CGroup {
	friend class CNetLibrary;
public:
	CGroup();
	//CGroup(int groupId);
	CGroup(int groupId, int mode);
	CGroup(int groupId, int mode, int fps);
	~CGroup();


public:

	void JoinGroup(unsigned __int64 sessionId, SSession* sessionPtr);
	virtual void ExitGroup(unsigned __int64 sessionId);
	virtual void ExitGroup(unsigned __int64 sessionId, int mode);
	void ExitGroup(unsigned __int64 sessionId, SSession* sessionPtr);
	void ExitGroup(SSession* sessionPtr);


	virtual void OnClientGroupJoin(unsigned __int64 sessionId) = 0;
	virtual void OnClientGroupExit(unsigned __int64 sessionId) = 0;

	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packet) = 0;
	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packetArray[], int size) = 0;

	void PushIndex(int i);
	long indexSize();
	static unsigned int GroupThread(LPVOID param);


	long GetFrame();

	int FindIndex(unsigned __int64);
	SSession* FindSessionPtr(int idx);



	CNetLibrary* netLib;

private:

	// 변경 필요.
	CLFQueue<SJob> jobQ;
	std::stack<int> indexAllocator;

	std::unordered_map<unsigned __int64, SSession*> groupSession;

	std::unordered_map<unsigned __int64, unsigned __int64> index;

	long fps;
	int groupId;
	int mode;
	HANDLE hGroupThread;
	HANDLE hJobEvent;
	HANDLE hExitEvent;
	ExtendSSession sessionPtrArrays[15000];



	/*long tryConnectMode;
	long frame;
	long connectMode;*/
};

class CGameServerInterface;

class CLobby : public CGroup {

public:
	/*friend class CNetLibrary;*/
	//friend class CGameServerInterface;
	//CLobby();
	CLobby(void* ptr);

	//virtual void ExitGroup(unsigned __int64 sessionId);

	virtual void OnClientGroupJoin(unsigned __int64 sessionId);
	virtual void OnClientGroupExit(unsigned __int64 sessionId);
	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packet);
	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packetArray[], int size);
	
	void RequestLogin(unsigned __int64 sessionId, CPacket* packetPtr);

	/*
	long GetGroupPlayerCount();
	long GetGroupPlayerMapCount();*/

	std::unordered_map<unsigned __int64, SPlayer*> groupPlayer;
	std::unordered_map<unsigned __int64, unsigned __int64> accountKey;
	CGameServerInterface* gameFace; // TODO????
	long mCount;
	//long playerCount;
};

class CField : public CGroup {

public:
	/*friend class CNetLibrary;;*/
	//friend class CGameServerInterface;
	//CField();
	CField(void* ptr);


	virtual void OnClientGroupJoin(unsigned __int64 sessionId);
	virtual void OnClientGroupExit(unsigned __int64 sessionId);
	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packet);
	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packetArray[], int size);


	inline void Echo(unsigned __int64 sessionId, CPacket* packet);
	
	/*
	long GetGroupPlayerCount();
	long GetGroupPlayerMapCount();*/

	alignas(64) std::unordered_map<unsigned __int64, SPlayer*> groupPlayer;
	alignas(64) std::unordered_map<unsigned __int64, unsigned __int64> accountKey;
	alignas(64) CGameServerInterface* gameFace; // TODO????
	//long playerCount;
};


extern unsigned __int64 testArrayIdx;
extern unsigned __int64 testArrayIdx2;
extern unsigned __int64 testArrayIdx3;