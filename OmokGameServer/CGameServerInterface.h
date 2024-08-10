#pragma once

//class CField : public CGroup {
//	friend class CNetLibrary;
//public:
//
//	virtual void OnClientGroupJoin(unsigned __int64 sessionId)
//	{
//		// Group 할당.
//		SPlayer* newPlayer;
//
//		newPlayer = playerPool.Alloc();
//
//	}
//	virtual void OnClientGroupExit(unsigned __int64 sessionId)
//	{
//		SPlayer* player;
//
//		playerPool.Free(player);
//	}
//	virtual void OnMessage(unsigned __int64 sessionId, CPacket* packet)
//	{
//		WORD type;
//
//		if (packet->Decoding() == 0)
//			DebugBreak();
//
//		*packet >> type;
//
//		switch (type)
//		{
//		case 5000: // en_PACKET_CS_GAME_SERVER
//			RequestEcho(sessionId, packet);
//			break;
//
//		default:
//			DebugBreak();
//			break;
//		}
//	}
//
//	void RequestEcho(unsigned __int64 sessionId, CPacket* packetPtr)
//	{
//		INT64 accountNo;
//		LONGLONG sendTick;
//
//		// data 뽑기.
//		*packetPtr >> accountNo >> sendTick;
//
//		// 전송...
//		packetPtr->Clear();
//		WORD type = 5001;
//		*packetPtr << type << accountNo;
//
//		// SendMessages();
//	}
//
//	CMemoryPool<SPlayer> playerPool{ 15000, false };
//	std::map<unsigned __int64, SPlayer*> groupPlayer;
//};


class CGameServerInterface : public CNetLibrary
{
public:
	CGameServerInterface();
//	CGameServerInterface();
	~CGameServerInterface();


	std::map<unsigned __int64, SPlayer*> playersMap;
	std::map<unsigned __int64, unsigned __int64> playersKey;
	SRWLOCK playerMapLock;


	virtual bool OnConnectionRequest(const WCHAR* IPAddress, USHORT port);
	virtual void OnClientJoin(unsigned __int64 sessionID);
	virtual void OnClientLeave(unsigned __int64 sessionID);
	virtual void OnError(int errorcode, const WCHAR* msg);


	SPlayer* FindPlayer(unsigned __int64 sessionId);


	static unsigned int MonitorThread(LPVOID param);


	CMemoryPool<SPlayer> playerPool;
	HANDLE hMonitorThread;



	void SRWELock();
	void SRWEUnLock();
	void SRWSLock();
	void SRWSUnLock();

	void ConnectToMonitorServer();

private:
	friend class CMonitorChatClient;
	CMonitorChatClient* monitorClientPtr;
};