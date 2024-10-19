#include <stdlib.h>

#include "CPacket.h"

CBucketPool<CPacket> CPacket::pool;

CPacket::CPacket()
	: iBufferSize(512), refCount(0), headerSize(2), encodeFlag(0)
{
	buffer = new char[iBufferSize];
	pPush = pPop = buffer + headerSize;
	checksum = pPush - 1;
}

CPacket::~CPacket()
{
	delete[] buffer;
}

void	CPacket::Clear(void)
{
	pPush = pPop = buffer + headerSize;
	encodeFlag = 0;
}

int		CPacket::MoveWritePos(int iSize)
{
	if (iSize <= 0)
		return 0;
	pPush += iSize;
	return iSize;
}

int		CPacket::MoveReadPos(int iSize)
{
	if (iSize <= 0)
		return 0;
	pPop += iSize;
	return iSize;
}

int		CPacket::GetData(char* chpDest, int iSrcSize)
{
	if (iSrcSize <= 0 || pPop + iSrcSize > buffer + iBufferSize)
		DebugBreak();
	memcpy(chpDest, pPop, iSrcSize);
	pPop += iSrcSize;

	return iSrcSize;
}

void		CPacket::GetCheckData(char* chpDest, int iSrcSize)
{
	if (iSrcSize <= 0 || buffer + iSrcSize > buffer + iBufferSize)
		DebugBreak();
	memcpy(chpDest, buffer, iSrcSize);
}


int		CPacket::PutData(const char* chpSrc, int iSrcSize)
{
	if (iSrcSize <= 0 || pPush + iSrcSize > buffer + iBufferSize)
		DebugBreak();
	memcpy(pPush, chpSrc, iSrcSize);
	pPush += iSrcSize;

	return iSrcSize;
}

CPacket& CPacket::operator << (bool value)
{
	*(bool*)pPush = value;
	pPush += sizeof(bool);

	return (*this);
}

CPacket& CPacket::operator << (char value)
{
	*(char*)pPush = value;
	pPush += sizeof(char);

	return (*this);
}

CPacket& CPacket::operator << (unsigned char value)
{
	*(unsigned char*)pPush = value;
	pPush += sizeof(unsigned char);

	return (*this);
}

CPacket& CPacket::operator << (short value)
{
	*(short*)pPush = value;
	pPush += sizeof(short);

	return (*this);
}

CPacket& CPacket::operator << (unsigned short value)
{
	*(unsigned short*)pPush = value;
	pPush += sizeof(unsigned short);

	return (*this);
}

CPacket& CPacket::operator << (int value)
{
	*(int*)pPush = value;
	pPush += sizeof(int);

	return (*this);
}

CPacket& CPacket::operator << (unsigned int value)
{
	*(unsigned int*)pPush = value;
	pPush += sizeof(unsigned int);

	return (*this);
}

CPacket& CPacket::operator << (long value)
{
	*(long*)pPush = value;
	pPush += sizeof(long);

	return (*this);
}

CPacket& CPacket::operator << (unsigned long value)
{
	*(unsigned long*)pPush = value;
	pPush += sizeof(unsigned long);

	return (*this);
}

CPacket& CPacket::operator << (__int64 value)
{
	if (pPush + sizeof(__int64) > buffer + iBufferSize)
		DebugBreak();
	*(__int64*)pPush = value;
	pPush += sizeof(__int64);

	return (*this);
}

CPacket& CPacket::operator << (unsigned __int64 value)
{
	if (pPush + sizeof(unsigned __int64) > buffer + iBufferSize)
		DebugBreak();
	*(unsigned __int64*)pPush = value;
	pPush += sizeof(unsigned __int64);

	return (*this);
}

CPacket& CPacket::operator << (float value)
{
	*(float*)pPush = value;
	pPush += sizeof(float);

	return (*this);
}

CPacket& CPacket::operator << (double value)
{
	*(double*)pPush = value;
	pPush += sizeof(double);

	return (*this);
}

CPacket& CPacket::operator >> (bool& value)
{
	value = *(bool*)(pPop);
	pPop += sizeof(bool);

	return (*this);
}

CPacket& CPacket::operator >> (char& value)
{
	value = *(char*)(pPop);
	pPop += sizeof(char);

	return (*this);
}

CPacket& CPacket::operator >> (unsigned char& value)
{
	value = *(unsigned char*)(pPop);
	pPop += sizeof(unsigned char);
	return (*this);
}

CPacket& CPacket::operator >> (short& value)
{
	value = *(short*)(pPop);
	pPop += sizeof(short);

	return (*this);
}

CPacket& CPacket::operator >> (unsigned short& value)
{
	value = *(unsigned short*)(pPop);
	pPop += sizeof(unsigned short);

	return (*this);
}

CPacket& CPacket::operator >> (int& value)
{
	value = *(int*)(pPop);
	pPop += sizeof(int);

	return (*this);
}

CPacket& CPacket::operator >> (unsigned int& value)
{
	value = *(unsigned int*)(pPop);
	pPop += sizeof(unsigned int);

	return (*this);
}

CPacket& CPacket::operator >> (long& value)
{
	value = *(long*)(pPop);
	pPop += sizeof(long);

	return (*this);
}

CPacket& CPacket::operator >> (unsigned long& value)
{
	value = *(unsigned long*)(pPop);
	pPop += sizeof(unsigned long);

	return (*this);
}

CPacket& CPacket::operator >> (__int64& value)
{
	value = *(__int64*)(pPop);
	pPop += sizeof(__int64);

	return (*this);
}

CPacket& CPacket::operator >> (unsigned __int64& value)
{
	if (pPop + sizeof(unsigned __int64) > buffer + iBufferSize)
		DebugBreak();
	value = *(unsigned __int64*)(pPop);
	pPop += sizeof(unsigned __int64);

	return (*this);
}

CPacket& CPacket::operator >> (float& value)
{
	value = *(float*)(pPop);
	pPop += sizeof(float);

	return (*this);
}

CPacket& CPacket::operator >> (double& value)
{
	value = *(double*)(pPop);
	pPop += sizeof(double);

	return (*this);
}

int CPacket::SetHeader(char* src, int pos, int size)
{
	if (size <= 0 || pos + size > headerSize)
		return 0;

	memcpy(buffer + pos, src, size);

	return size;
}

int CPacket::GetHeader(char* dest, int pos, int size)
{
	if (size <= 0 || pos + size > headerSize)
		return 0;

	memcpy(dest, buffer + pos, size);

	return size;
}

int CPacket::Encoding()
{
	unsigned char p, e;
	unsigned char randKey;
	unsigned char constKey;
	unsigned short payloadSize;
	long ret;
	char* buf = GetCheckPtr();
	// -----

	ret = InterlockedExchange(&encodeFlag, 1);
	if (ret == 1)
		return (false);

	// 인코딩 작업 초기화
	p = 0;
	e = 0;
	randKey = *(buffer + 3);
	constKey = 0x32;
	payloadSize = *(unsigned short*)(buffer + 1);

	/*printf("Prev\n");
	for (int i = 0; i <= payloadSize; ++i)
	{
		printf("[0x%02x]", *(buf + i));
	}*/

	for (unsigned short i = 0; i <= payloadSize; ++i)
	{
		p = *(buf + i) ^ (p + randKey + (1 + i));
		e = p ^ (e + constKey + (unsigned char)(1 + i));
		*(buf + i) = e;
	}
	/*printf("After\n");
	for (int i = 0; i <= payloadSize; ++i)
	{
		printf("[0x%02x]", *(unsigned char *)(buf + i));
	}
	printf("\n");*/
	return (true);
}

int CPacket::Decoding()
{
	unsigned char checkSum;
	unsigned char p;
	unsigned char d;
	unsigned char randKey;
	unsigned char constKey;
	unsigned short payloadSize;
	unsigned char pe = 0, pp = 0;
	// -----

	p = 0;
	d = 0;
	checkSum = *(buffer + 4);
	randKey = *(buffer + 3);
	constKey = 0x32;
	payloadSize = *(short*)(buffer + 1);

	char* buf = GetCheckPtr();
	// 현재p =  현재 e ^ (이전 e + k + ?)
	// 현재d = 
	for (unsigned short i = 0; i <= payloadSize; ++i)
	{
		p = buf[i] ^ (pe + constKey + (unsigned char)(1 + i));

		d = p ^ (pp + randKey + (unsigned char)(1 + i));
		pe = buf[i];
		pp = p;
		buf[i] = d;
	}
	//printf("\n");
	unsigned char cmp = 0;
	for (int i = 1; i <= payloadSize; ++i)
	{
		cmp += buf[i];
		//printf("[0x%02x]", buf[i]);
	}
	cmp %= 256;
	checkSum = *(buffer + 4);
	if (checkSum != cmp)
	{
		//wprintf(L"Decode Error\n");
		return false;
	}
	/*for (int i = 0; i <= payloadSize; ++i)
	{
		printf("[0x%02x]", buf[i]);
	}*/

	return true;
}

CPacket* InitPacket()
{
	
	auto pNewPacket = CPacket::Alloc();
	auto refCnt = pNewPacket->AddRef();	// type(long)
	if (refCnt != 1)
		DebugBreak();

	return pNewPacket;
	
}