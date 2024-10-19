#pragma comment(lib,"Pdh.lib")

#include <Pdh.h>
#include <stdio.h>
#include <strsafe.h>
#include <Windows.h>
#include <string>
#include "CPDH.h"

// Performance Data Helper Class를 이용하여, 원하는 값을 얻는 class임

// network
CNetworkPDH::CNetworkPDH()
{
	int iCnt = 0;
	PDH_STATUS code;
	// -----

	code = PdhOpenQuery(NULL, NULL, &networkRecvQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}

	code = PdhOpenQuery(NULL, NULL, &networkSendQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}

	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);
	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];
	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		delete[] szCounters;
		delete[] szInterfaces;
		DebugBreak();
	}
	iCnt = 0;
	szCur = szInterfaces;
	//---------------------------------------------------------
	// szInterfaces 에서 문자열 단위로 끊으면서 , 이름을 복사받는다.
	//---------------------------------------------------------
	for (; *szCur != L'\0' && iCnt < en_ETHERNET_MAX; szCur += wcslen(szCur) + 1, iCnt++)
	{
		_EthernetStruct[iCnt]._bUse = true;
		_EthernetStruct[iCnt]._szName[0] = L'\0';

		wcscpy_s(_EthernetStruct[iCnt]._szName, szCur);

		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Received/sec", szCur);
		code = PdhAddCounter(networkRecvQuery, szQuery, NULL, &_EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes);
		if (code != ERROR_SUCCESS)
			DebugBreak();

		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Sent/sec", szCur);
		code = PdhAddCounter(networkSendQuery, szQuery, NULL, &_EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes);
		if (code != ERROR_SUCCESS)
			DebugBreak();
	}
}

long CNetworkPDH::GetNetworkRecvBytes()
{
	PDH_FMT_COUNTERVALUE counterValue;
	PDH_STATUS Status;
	int idx;
	// -----

	totalNetworkRecvBytes = 0;
	PdhCollectQueryData(networkRecvQuery);
	
	for (idx = 0; idx < en_ETHERNET_MAX; idx++)
	{
		if (_EthernetStruct[idx]._bUse)
		{
			Status = PdhGetFormattedCounterValue(_EthernetStruct[idx]._pdh_Counter_Network_RecvBytes,
				PDH_FMT_LARGE, NULL, &counterValue);
			if (Status == 0)
				totalNetworkRecvBytes += counterValue.largeValue;
		}
	}

	return totalNetworkRecvBytes;
}

long CNetworkPDH::GetNetworkRecvKBytes()
{
	return GetNetworkRecvBytes() / 1024;
}

long CNetworkPDH::GetNetworkRecvMBytes()
{

	return GetNetworkRecvKBytes() / 1024;
}

long CNetworkPDH::GetNetworkRecvGBytes()
{
	return GetNetworkRecvMBytes() / 1024;
}

long CNetworkPDH::GetNetworkSendBytes()
{
	PDH_FMT_COUNTERVALUE counterValue;
	PDH_STATUS Status;
	int idx;
	// -----

	totalNetworkSendBytes = 0;
	PdhCollectQueryData(networkSendQuery);

	for (idx = 0; idx < en_ETHERNET_MAX; idx++)
	{
		if (_EthernetStruct[idx]._bUse)
		{
			Status = PdhGetFormattedCounterValue(_EthernetStruct[idx]._pdh_Counter_Network_SendBytes,
				PDH_FMT_LARGE, NULL, &counterValue);
			if (Status == 0)
				totalNetworkSendBytes += counterValue.largeValue;
		}
	}

	return totalNetworkSendBytes;
}

long CNetworkPDH::GetNetworkSendKBytes()
{
	return GetNetworkSendBytes() / 1024;
}

long CNetworkPDH::GetNetworkSendMBytes()
{
	return GetNetworkSendKBytes() / 1024;
}

long CNetworkPDH::GetNetworkSendGBytes()
{
	return GetNetworkSendMBytes() / 1024;
}

CMemoryPDH::CMemoryPDH()
{
	PDH_STATUS code;
	// -----

	code = PdhOpenQuery(NULL, NULL, &memoryAvailableQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}

	code = PdhOpenQuery(NULL, NULL, &memoryNPQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}

	code = PdhOpenQuery(NULL, NULL, &memoryPQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}


	code = PdhAddCounter(memoryAvailableQuery, L"\\Memory\\Available MBytes", NULL, &memoryAvailableMbyte);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhAddCounter(memoryNPQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &memoryNPbyte);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhAddCounter(memoryPQuery, L"\\Memory\\Pool Paged Bytes", NULL, &memoryPbyte);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
}

long CMemoryPDH::GetAvailableMBytes()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----
	
	PdhCollectQueryData(memoryAvailableQuery);

	if (PdhGetFormattedCounterValue(memoryAvailableMbyte, PDH_FMT_LARGE, NULL, &counterValue) != ERROR_SUCCESS)
	{
		wprintf(L"GetAvailableMBytes Code:%d\n", GetLastError());
		DebugBreak();
	}
	return counterValue.largeValue;
}

long CMemoryPDH::GetNPBytes()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(memoryNPQuery);

	if (PdhGetFormattedCounterValue(memoryNPbyte, PDH_FMT_LARGE, NULL, &counterValue) != ERROR_SUCCESS)
	{
		wprintf(L"GetNPBytes Code:%d\n", GetLastError());
		DebugBreak();
	}
	return counterValue.largeValue;
}

long CMemoryPDH::GetPBytes()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(memoryPQuery);

	if (PdhGetFormattedCounterValue(memoryPbyte, PDH_FMT_LARGE, NULL, &counterValue) != ERROR_SUCCESS)
	{
		wprintf(L"GetPBytes Code:%d\n", GetLastError());
		DebugBreak();
	}
	return counterValue.largeValue;
}

// -------------------------------------------------------------

CProcessPDH::CProcessPDH(const std::wstring& filename)
{
	PDH_STATUS code;
	std::wstring pdh;
	// -----

	wprintf(L"filename:%s\n", filename.c_str());
	
	code = PdhOpenQuery(NULL, NULL, &processHandleQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}

	code = PdhOpenQuery(NULL, NULL, &processThreadQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhOpenQuery(NULL, NULL, &processUserCpuQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhOpenQuery(NULL, NULL, &processTotalCpuQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhOpenQuery(NULL, NULL, &processNPQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhOpenQuery(NULL, NULL, &processPQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhOpenQuery(NULL, NULL, &processPrivateQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}
	code = PdhOpenQuery(NULL, NULL, &processWorkingPrivateQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}


	pdh = L"\\Process(" + filename + L")\\Handle Count";
	code = PdhAddCounter(processHandleQuery, pdh.c_str(), NULL, &processHandleCount);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\Thread Count";
	code = PdhAddCounter(processThreadQuery, pdh.c_str(), NULL, &processThreadCount);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\% User Time";
	code = PdhAddCounter(processUserCpuQuery, pdh.c_str(), NULL, &processUserCpuUsage);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\% Processor Time";
	code = PdhAddCounter(processTotalCpuQuery, pdh.c_str(), NULL, &processTotalCpuUsage);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\Pool Paged Bytes";
	code = PdhAddCounter(processPQuery, pdh.c_str(), NULL, &processP);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\Pool Nonpaged Bytes";
	code = PdhAddCounter(processNPQuery, pdh.c_str(), NULL, &processNP);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\Private Bytes";
	code = PdhAddCounter(processPrivateQuery, pdh.c_str(), NULL, &processPrivateByte);
	if (code != ERROR_SUCCESS)
		DebugBreak();

	pdh = L"\\Process(" + filename + L")\\Working Set - Private";
	code = PdhAddCounter(processWorkingPrivateQuery, pdh.c_str(), NULL, &processWorkingPrivateByte);
	if (code != ERROR_SUCCESS)
		DebugBreak();
}


long CProcessPDH::GetHandleCount()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processHandleQuery);

	PdhGetFormattedCounterValue(processHandleCount, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}
long CProcessPDH::GetThreadCount()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processThreadQuery);

	PdhGetFormattedCounterValue(processThreadCount, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}

long CProcessPDH::GetUserCpuInteger()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processUserCpuQuery);

	PdhGetFormattedCounterValue(processUserCpuUsage, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}


long CProcessPDH::GetTotalCpuInteger()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processTotalCpuQuery);

	PdhGetFormattedCounterValue(processTotalCpuUsage, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}

float CProcessPDH::GetUserCpuFloat()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processUserCpuQuery);

	PdhGetFormattedCounterValue(processUserCpuUsage, PDH_FMT_DOUBLE, NULL, &counterValue);

	return counterValue.doubleValue;
}


float CProcessPDH::GetTotalCpuFloat()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processTotalCpuQuery);

	PdhGetFormattedCounterValue(processTotalCpuUsage, PDH_FMT_DOUBLE, NULL, &counterValue);

	return counterValue.doubleValue;
}


long CProcessPDH::GetNonPaged()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processNPQuery);

	PdhGetFormattedCounterValue(processNP, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}
long CProcessPDH::GetPaged()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processPQuery);

	PdhGetFormattedCounterValue(processP, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}
long CProcessPDH::GetPrivateMem()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processPrivateQuery);

	PdhGetFormattedCounterValue(processPrivateByte, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}

long CProcessPDH::GetWorkingPrivateMem()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(processWorkingPrivateQuery);

	PdhGetFormattedCounterValue(processWorkingPrivateByte, PDH_FMT_LONG, NULL, &counterValue);

	return counterValue.longValue;
}


CProcessorPDH::CProcessorPDH()
{
	PDH_STATUS code;
	// -----

	code = PdhOpenQuery(NULL, NULL, &cpuQuery);
	if (code != ERROR_SUCCESS)
	{
		wprintf(L"PdhOpenQuery_processQuery : %d\n", code);
		DebugBreak();
	}

	code = PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuUsage);
	if (code != ERROR_SUCCESS)
		DebugBreak();
}

float CProcessorPDH::GetTotalCpuUsage()
{
	PDH_FMT_COUNTERVALUE counterValue;
	// -----

	PdhCollectQueryData(cpuQuery);

	PdhGetFormattedCounterValue(cpuUsage, PDH_FMT_DOUBLE, NULL, &counterValue);

	return counterValue.doubleValue;
}
