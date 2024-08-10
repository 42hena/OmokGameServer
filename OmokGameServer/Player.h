#pragma once

struct SPlayer
{
	SPlayer()
		: mode(0),
		packetCount(0),
		accountNo(-1)
	{}

	/*SPlayer(unsigned __int64 sessionId)
		: mode(0),
		sessionId(sessionId)
	{}*/
	unsigned __int64 sessionId;
	long mode;
	CPacket* packets[200];
	long packetCount;
	long accountNo;
};