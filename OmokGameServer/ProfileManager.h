#ifndef __PROFILE_MANAGER_H__
#define __PROFILE_MANAGER_H__

#include <Windows.h>

class CProfileManager;

extern CProfileManager g_profileManager;

// ##################################################
// SProfilerData (struct)
// ##################################################
// 원하는 위치를 측정하고, 저장하는 방식으로 사용시간 
// (average, min, max) 값과 사용 빈도를 나타내기 위한
// 구조체
// ##################################################

struct SProfilerData
{
// default 함수
public:
	SProfilerData();

// Member variable
public:
	bool flag = false;
	wchar_t name[50];
	__int64 runningTime;
	LARGE_INTEGER beginTime;
	__int64 maxTime[2];
	__int64 minTime[2];
	__int64 callCount;
	int arrayIndex;
};

void ProfileBegin(const wchar_t* name);
void ProfileEnd(const wchar_t* name);
void ProfileDataOutToText();

class CProfiler
{
public:
	CProfiler(const wchar_t* name)
		: tag(name)
	{
		ProfileBegin(name);
	}
	~CProfiler()
	{
		ProfileEnd(tag);
	}

private:
	const wchar_t* tag;
};

#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
#define PRO_END(TagName)	ProfileEnd(TagName)

class CProfileThread
{
// function
public:
	CProfileThread();
	~CProfileThread();

	CProfileThread( const CProfileThread& copy ) = delete;
	CProfileThread& operator=( const CProfileThread& copy ) = delete;

public:
	bool GetFlag(int idx) const;
	__int64 GetCallCount(int idx) const;
	const wchar_t* GetTagName(int idx) const;


	void UseData(int idx, const wchar_t* tagName);
	void StartRecord(int idx);

	__int64 GetStartTime(int idx);

	void AddRunTime(int idx, __int64 operatingTime);
	void ChangeMaxValue(int idx, __int64 operatingTime);
	void ChangeMinValue(int idx, __int64 operatingTime);

	__int64 GetMaxValue(int idx);
	__int64 GetMinValue(int idx);
	__int64 GetTotalMaxValue(int idx);
	__int64 GetTotalMinValue(int idx);
	__int64 GetTotalRuntime(int idx)
	{
		return profileData[idx].runningTime;
	}
	DWORD GetThreadID()
	{
		return threadId;
	}
	void SetThreadID(DWORD tid)
	{
		threadId = tid;
	}

private:
	SProfilerData profileData[10];
	unsigned int threadId;
};

class CProfileManager
{
// OCF function
public:
	CProfileManager();
	~CProfileManager();

	CProfileManager( const CProfileManager& copy ) = delete;
	CProfileManager& operator=( const CProfileManager& copy ) = delete;

public:
	DWORD GetTlsIndex();
	DWORD GetProfileIndex();
	DWORD GetI()
	{
		return index;
	}
	CProfileThread& GetThread(int index)
	{
		return threadArray[index];
	}
	CProfileThread* GetProfile();
	
private:
	CProfileThread threadArray[20];
	alignas(64) long index;
	DWORD tlsIndex;
};

#define MAX_PROFILE_SIZE 10

#endif
