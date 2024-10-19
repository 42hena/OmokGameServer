#pragma once

class CNetworkPDH
{
public:
	enum en_PDH_LIMIT {
		en_ETHERNET_MAX = 8,

	};
	// 이더넷 하나에 대한 Send,Recv PDH 쿼리 정보.
	struct st_ETHERNET
	{
		bool _bUse;
		WCHAR _szName[128];
		PDH_HCOUNTER _pdh_Counter_Network_RecvBytes;
		PDH_HCOUNTER _pdh_Counter_Network_SendBytes;
	};
	st_ETHERNET _EthernetStruct[en_ETHERNET_MAX]; // 랜카드 별 PDH 정보
	
	uintptr_t totalNetworkRecvBytes;
	uintptr_t totalNetworkSendBytes;

public:
	CNetworkPDH();

public:
	long GetNetworkRecvBytes();
	long GetNetworkRecvKBytes();
	long GetNetworkRecvMBytes();
	long GetNetworkRecvGBytes();

	long GetNetworkSendBytes();
	long GetNetworkSendKBytes();
	long GetNetworkSendMBytes();
	long GetNetworkSendGBytes();


private:
	PDH_HQUERY networkRecvQuery;
	PDH_HQUERY networkSendQuery;
	bool bErr = false;
	WCHAR* szCur = NULL;
	WCHAR* szCounters = NULL;
	WCHAR* szInterfaces = NULL;
	DWORD dwCounterSize = 0;
	DWORD dwInterfaceSize = 0;
	WCHAR szQuery[1024] = { 0, };
};

class CMemoryPDH {
public:
	CMemoryPDH();

	long GetNPBytes();
	long GetPBytes();
	long GetAvailableMBytes();

private:
	PDH_HQUERY memoryAvailableQuery;
	PDH_HQUERY memoryNPQuery;
	PDH_HQUERY memoryPQuery;

	PDH_HCOUNTER memoryAvailableMbyte;
	PDH_HCOUNTER memoryNPbyte;
	PDH_HCOUNTER memoryPbyte;
};

class CProcessPDH {
public:
	CProcessPDH(const std::wstring &filename);
	
	long GetHandleCount();
	long GetThreadCount();
	long GetUserCpuInteger();
	long GetTotalCpuInteger();
	float GetUserCpuFloat();
	float GetTotalCpuFloat();
	long GetNonPaged();
	long GetPaged();
	long GetPrivateMem();
	long GetWorkingPrivateMem();


private:
	PDH_HQUERY processHandleQuery;
	PDH_HQUERY processThreadQuery;
	PDH_HQUERY processUserCpuQuery;
	PDH_HQUERY processTotalCpuQuery;
	PDH_HQUERY processNPQuery;
	PDH_HQUERY processPQuery;
	PDH_HQUERY processPrivateQuery;
	PDH_HQUERY processWorkingPrivateQuery;
	

	PDH_HCOUNTER processHandleCount;
	PDH_HCOUNTER processThreadCount;
	PDH_HCOUNTER processUserCpuUsage;
	PDH_HCOUNTER processTotalCpuUsage;
	PDH_HCOUNTER processNP;
	PDH_HCOUNTER processP;
	PDH_HCOUNTER processPrivateByte;
	PDH_HCOUNTER processWorkingPrivateByte;
};

class CProcessorPDH {
public:
	CProcessorPDH();

	float GetTotalCpuUsage();
	

private:
	PDH_HQUERY cpuQuery;

	PDH_HCOUNTER cpuUsage;
};
