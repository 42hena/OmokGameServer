#include <process.h>
#include <unordered_map>
#include <map>

#include <Windows.h>


#include "CPacket.h"
#include "CRBuffer.h"
#include "CLFQueue.h"
#include "CLFStack.h"
#include "Session.h"

#include "Job.h"


#include "Player.h"
#include "CGroup.h"
#include "CGroupManager.h"
#include "CNetLibrary.h"



//CGroupManager::CGroupManager()
//{
//
//}

CGroupManager::CGroupManager(void *ptr)
	: lobby(ptr), field(ptr)
{
	netLib = reinterpret_cast<CNetLibrary*>(ptr);
}

CGroupManager::~CGroupManager()
{

}

void CGroupManager::MoveDefault(unsigned __int64 sessionId, SSession* sessionPtr)
{
	long ret = InterlockedExchange(&sessionPtr->mode, 1);
	if (ret != 0)
		DebugBreak();
	lobby.JoinGroup(sessionId, sessionPtr);
}

void CGroupManager::AuthSuccess(unsigned __int64 sessionId, SSession* sessionPtr)
{
	field.JoinGroup(sessionId, sessionPtr);
}