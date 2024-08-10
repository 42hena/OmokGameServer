#include "CRedisDBManager.h"

CRedisDBManager::CRedisDBManager()
	: dbIndex(-1),
		maxCount(10) // const
{
	int i;
// -----
	tlsIndex = TlsAlloc();
	if (tlsIndex == TLS_OUT_OF_INDEXES)
	{
		DebugBreak();
	}

	for (i = 0; i < 10; ++i)
	{
		client[i].connect();
	}
	wprintf(L"qwer\n");
}

CRedisDBManager::CRedisDBManager(const std::string& ip, size_t port)
	: dbIndex(-1),
		maxCount(10) // const
{
	int i;
// -----
	
	tlsIndex = TlsAlloc();
	if (tlsIndex == TLS_OUT_OF_INDEXES)
	{
		DebugBreak();
	}

	for (i = 0; i < 10; ++i)
	{
		client[i].connect(ip, port);
	}
}

CRedisDBManager::~CRedisDBManager()
{
	int i;
// -----

	for (i = 0; i < 10; ++i)
	{
		client[i].disconnect();
	}
}

cpp_redis::client* CRedisDBManager::RedisDBAlloc()
{
	void* ptr;
	DWORD ret;
	// -----

	if ((ptr = TlsGetValue(tlsIndex)) == nullptr)
	{
		ret = InterlockedAdd((long*)&dbIndex, 1);
		if (ret == 10)
			DebugBreak();
		TlsSetValue(tlsIndex, &client[ret]);
		return &client[ret];
	}
	return (cpp_redis::client*)ptr;
}

void CRedisDBManager::RedisDBFree()
{
	void* ret;

	if ((ret = TlsGetValue(tlsIndex)) == nullptr)
	{
		DebugBreak();
	}
}