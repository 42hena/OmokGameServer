#pragma once
#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")


#include <cpp_redis/cpp_redis>
#include <CBucketPool.h>

class CRedisDBManager {
public:
	CRedisDBManager();
	CRedisDBManager(const std::string& ip, size_t port);
	~CRedisDBManager();

public:
	cpp_redis::client* RedisDBAlloc();
	void RedisDBFree();

private:	// 삭제 함수.
	CRedisDBManager(const CRedisDBManager& c) = delete;
	bool operator=(const CRedisDBManager& c) = delete;

private:
	cpp_redis::client client[10];
	DWORD tlsIndex;
	DWORD dbIndex;
	const int maxCount; // Init in [constructor]
};
