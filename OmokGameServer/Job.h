#pragma once

enum class en_JOB_TYPE : DWORD
{
	en_JobOnAccept = 0,
	en_JobOnMessage = 1,
	en_JobOnRelease = 2,
	en_JobServerDown = 3,
	en_JobSystem = 4,
};


struct SJob
{
	SJob()
	{

	}
	SJob(en_JOB_TYPE packetType, unsigned __int64 sessionId, CPacket* pPacket)
		: type(packetType), sessionId(sessionId), data(pPacket)
	{

	}
	en_JOB_TYPE type;
	unsigned __int64 sessionId;
	CPacket* data;
};
