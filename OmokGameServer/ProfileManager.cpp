#include <iostream>
#include <Windows.h>

#include "ProfileManager.h"

// SProfilerData 구조체 생성자
SProfilerData::SProfilerData()
	: flag(false), 
	runningTime(0),
	beginTime{ 0, 0 },
	maxTime{ 0, 0 },
	minTime{ 0x7fffffffffffffff, 0x7fffffffffffffff },
	callCount(0),
	arrayIndex(0)
{
	name[0] = 0;
#if defined(MY_DEBUG) && defined(SPROFILER_DATA_DEBUG)
	wprintf(L"SProfilerData ctor\n");
#endif
}

// ----------------------------------------------------------------------------------------------------

CProfileThread::CProfileThread()
	: threadId(0)
{
#if defined(MY_DEBUG) && defined(CProfileThread_DEBUG)
	wprintf(L"CProfileThread ctor\n");
#endif
}

CProfileThread::~CProfileThread()
{
#if defined(MY_DEBUG) && defined(CProfileThread_DEBUG)
	wprintf(L"CProfileThread dtor\n");
#endif
}

bool CProfileThread::GetFlag(int idx) const
{
	return (profileData[idx].flag);
}

__int64 CProfileThread::GetCallCount(int idx) const
{
	return profileData[idx].callCount;
}

const wchar_t* CProfileThread::GetTagName(int idx) const
{
	return (profileData[idx].name);
}

void CProfileThread::UseData(int idx, const wchar_t* tagName)
{
	wcscpy_s(profileData[idx].name, tagName);
	profileData[idx].flag = true;

	//wcscpy_s(profileThread->profileData[destIndex].name, name);
	//profileThread->profileData[destIndex].flag = true;
}

void CProfileThread::StartRecord(int idx)
{
	/*QueryPerformanceCounter(&profileThread->profileData[destIndex].beginTime);
	profileThread->profileData[destIndex].callCount++;*/
	QueryPerformanceCounter(&profileData[idx].beginTime);
	profileData[idx].callCount++;
}

__int64 CProfileThread::GetStartTime(int idx)
{
	return profileData[idx].beginTime.QuadPart;
}

void CProfileThread::AddRunTime(int idx, __int64 operatingTime)
{
	profileData[idx].runningTime += operatingTime;
}

void CProfileThread::ChangeMaxValue(int idx, __int64 operatingTime)
{
	if (profileData[idx].maxTime[0] > profileData[idx].maxTime[1])
	{
		profileData[idx].maxTime[1] = operatingTime;
	}
	else
	{
		profileData[idx].maxTime[0] = operatingTime;
	}
}

void CProfileThread::ChangeMinValue(int idx, __int64 operatingTime)
{
	if (profileData[idx].minTime[0] < profileData[idx].minTime[1])
	{
		profileData[idx].minTime[1] = operatingTime;
	}
	else
	{
		profileData[idx].minTime[0] = operatingTime;
	}
}

__int64 CProfileThread::GetMaxValue(int idx)
{
	if (profileData[idx].maxTime[0] > profileData[idx].maxTime[1])
		return profileData[idx].maxTime[0];
	else
		return profileData[idx].maxTime[1];
	//return std::max(profileData[idx].minTime[0], profileData[idx].minTime[1]);
	//return 1;
}

__int64 CProfileThread::GetMinValue(int idx)
{
	if (profileData[idx].minTime[0] < profileData[idx].minTime[1])
		return profileData[idx].minTime[0];
	else
		return profileData[idx].minTime[1];
	//return 0;
}

__int64 CProfileThread::GetTotalMaxValue(int idx)
{
	__int64 total;
	// -----

	total = 0;
	for (int i = 0; i < 2; ++i)
		total += profileData[idx].maxTime[i];
	return total;
}

__int64 CProfileThread::GetTotalMinValue(int idx)
{
	__int64 total;
	// -----

	total = 0;
	for (int i = 0; i < 2; ++i)
		total += profileData[idx].minTime[i];
	return total;
}

// ----------------------------------------------------------------------------------------------------

CProfileManager g_profileManager;

CProfileManager::CProfileManager()
	: index(-1)
{
	tlsIndex = TlsAlloc();

	//TlsAlloc이 실패 시 에러 처리
	if (tlsIndex == TLS_OUT_OF_INDEXES)
	{
		wprintf(L"In CProfileManager, tlsIndex is [TLS_OUT_OF_INDEXES]\n");
		exit(0);
	}
#if defined(MY_DEBUG) && defined(CPROFILE_MANAGER_DEBUG)
	wprintf(L"tlsIndex is %d\n", tlsIndex);
#endif
}

CProfileManager::~CProfileManager()
{
	TlsFree(tlsIndex);
#if defined(MY_DEBUG) && defined(CPROFILE_MANAGER_DEBUG)
	wprintf(L"CProfileManager dtor\n");
#endif
}

DWORD CProfileManager::GetTlsIndex()
{
	return tlsIndex;
}

DWORD CProfileManager::GetProfileIndex()
{
	long prevIndex;
// -----

	prevIndex = InterlockedAdd(&index, 1);
	threadArray[prevIndex].SetThreadID(GetCurrentThreadId());
	return prevIndex;
}

CProfileThread* CProfileManager::GetProfile()
{
	return &threadArray[GetProfileIndex()];
}

// ---

void ProfileBegin(const wchar_t* name)
{
	int destIndex;
	int i;								// only loop
	bool flag;
	DWORD tlsIndex;
	LPVOID ptr;
	CProfileThread* profileThread;
	// -----

		// Init local variable
	destIndex = 0;
	flag = false;
	tlsIndex = g_profileManager.GetTlsIndex();

	ptr = TlsGetValue(tlsIndex);
	if (ptr == nullptr)
	{
		profileThread = g_profileManager.GetProfile();
		TlsSetValue(tlsIndex, profileThread);
	}
	else
		profileThread = (CProfileThread*)ptr;

	// Find empty array
	for (i = 0; i < MAX_PROFILE_SIZE; ++i)
	{
		// if ( (profileThread->profileData[i].flag == false) || !wcscmp(name, profileThread->profileData[i].name))
		if ((profileThread->GetFlag(i) == false) || !wcscmp(name, profileThread->GetTagName(i)))
		{
			destIndex = i;
			break;
		}
	}

	// First attempt
	//if (profileThread->profileData[destIndex].callCount == 0)
	if (profileThread->GetCallCount(destIndex) == 0)
	{
		//wcscpy_s(profileThread->profileData[destIndex].name, name);
		//profileThread->profileData[destIndex].flag = true;

		profileThread->UseData(destIndex, name);
	}

	// Start record time and call count
	profileThread->StartRecord(destIndex);
	/*QueryPerformanceCounter(&profileThread->profileData[destIndex].beginTime);
	profileThread->profileData[destIndex].callCount++;*/
}

void ProfileEnd(const wchar_t* name)
{
	int index;
	__int64 operatingTime;
	LARGE_INTEGER endTime;
	int i;
	CProfileThread* profileThread;
	DWORD tlsIndex;
	LPVOID ptr;
	// -----

		// Stop time
	QueryPerformanceCounter(&endTime);

	tlsIndex = g_profileManager.GetTlsIndex();

	ptr = TlsGetValue(tlsIndex);
	if (ptr == nullptr)
	{
		wprintf(L"ProfileEnd");
		return;
	}
	profileThread = (CProfileThread*)ptr;

	// Find dest time
	index = -1;
	for (i = 0; i < MAX_PROFILE_SIZE; ++i)
	{
		//if (!wcscmp(name, profileThread->profileData[i].name))
		if (!wcscmp(name, profileThread->GetTagName(i)))
		{
			index = i;
			break;
		}
	}

	// Error occur
	if (index == -1)
		return;

	// Get Operating Time
	//operatingTime = endTime.QuadPart - profileThread->profileData[index].beginTime.QuadPart;
	operatingTime = endTime.QuadPart - profileThread->GetStartTime(index);
	// Add Running Time
	profileThread->AddRunTime(index, operatingTime);

	// Change Max
	profileThread->ChangeMaxValue(index, operatingTime);

	// Change Min
	profileThread->ChangeMinValue(index, operatingTime);
}

void ProfileDataOutToText()
{
	char buf[256];
	FILE* fp;
	LARGE_INTEGER freq;
	SYSTEMTIME tm;
	int errCode;

	// Get
	QueryPerformanceFrequency(&freq);

	GetLocalTime(&tm);
	sprintf_s(buf, 256, "%4d_%02d_%02d_%02d_%02d_profile_log.txt", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute);
	

	errCode = fopen_s(&fp, buf, "ab+");
	if (errCode)
	{
		errCode = GetLastError();
		wprintf(L"ProfileDataOutText Fail %d\n", errCode);
		return;
	}

	for (DWORD idx = 0; idx <= g_profileManager.GetI(); ++idx)
	{
		sprintf_s(buf, 200, "THREAD|        Name        |       Average      |         Min        |          Max       |        Call        | \n");
		fwrite(buf, 1, strlen(buf), fp);

		CProfileThread& now = g_profileManager.GetThread(idx);
		for (int i = 0; i < MAX_PROFILE_SIZE; ++i)
		{
			//printf("%lld %lld %lld %lld\n", g_profileManager.threadArray[idx].profileData[i].minTime[0], g_profileManager.threadArray[idx].profileData[i].minTime[1], g_profileManager.threadArray[idx].profileData[i].maxTime[0], g_profileManager.threadArray[idx].profileData[i].maxTime[1]);
			if (now.GetFlag(i))
			{
				sprintf_s(buf, 200, "%6d|%-20ws|%18.6lfms|%18.6lfms|%18.6lfms|%20lld|\n",
					now.GetThreadID(),
					now.GetTagName(i), 
					(double)(now.GetTotalRuntime(i)
						- now.GetTotalMaxValue(i)
						- now.GetTotalMinValue(i)) / (now.GetCallCount(i) - 4) / freq.QuadPart * 1000,
					now.GetMinValue(i) / (double)freq.QuadPart * 1000,
					now.GetMaxValue(i) / (double)freq.QuadPart * 1000,
					now.GetCallCount(i));
				fwrite(buf, 1, strlen(buf), fp);
			}
			else
				break;
		}
	}
	fclose(fp);
}


// 문제 상황
// 24-01-18
// 1. 문자열 이니셜라이저 어떻게 하지?
// => 대안 1. 생성자 안에서 초기화
// => 대안 2. initialize 찾아보기.
// 2. 싱글톤 방식으로 바꿔보기.
// => 일단 실패했음.
