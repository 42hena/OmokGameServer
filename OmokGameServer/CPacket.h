#ifndef  __CPacket_H__
#define  __CPacket_H__
#include <Windows.h>
#include "CBucketPool.h"

struct aaa {
	uintptr_t id;
	int type;
	long count;
};

class CPacket
{
public:

	// 생성자
	CPacket();

	// 상속 대비 + 소멸자
	virtual ~CPacket();

	// ####################################################################################################
	// #                   Utils                                                                          #
	// ####################################################################################################

	// ##################################################
	// # 제일 처음 상태로 되돌리기                      #
	// #                                                #
	// # Param  : (None)                                # 
	// # return : (None)                                # 
	// ##################################################
	void	Clear(void);

	// ##################################################
	// # 최대 버퍼의 크기                               #
	// #                                                #
	// # Param  : (None)                                # 
	// # return : (int : Max Buffer size)               # 
	// ##################################################
	int	GetBufferSize(void) { return iBufferSize; }

	// ##################################################
	// # 현재 저장 중인 버퍼의 사이즈                   #
	// #                                                #
	// # Param  : (None)                                #
	// # return : (int : using Buffer size)             #
	// ##################################################
	__int64	GetDataSize(void) { return pPush - pPop; }

	// ##################################################
	// # 버퍼의 처음 위치를 반환                        #
	// #                                                #
	// # Param  : (None)                                #
	// # return : (char * : buffer_ptr)                 #
	// ##################################################
	char* GetBufferPtr(void) { return buffer; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int iSize);
	int		MoveReadPos(int iSize);

	int GetLastPtr(void) { return (int)(pPush - buffer); }


	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, int iSize);
	void	GetCheckData(char* chpDest, int iSrcSize);
	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char* chpSrc, int iSrcSize);


	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	//CPacket& operator = (CPacket& clSrCPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CPacket& operator << (bool);

	CPacket& operator << (char);
	CPacket& operator << (unsigned char);

	CPacket& operator << (short);
	CPacket& operator << (unsigned short);

	CPacket& operator << (int);
	CPacket& operator << (unsigned int);

	CPacket& operator << (long);
	CPacket& operator << (unsigned long);

	CPacket& operator << (__int64);
	CPacket& operator << (unsigned __int64);

	CPacket& operator << (float);
	CPacket& operator << (double);

	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CPacket& operator >> (bool&);

	CPacket& operator >> (char&);
	CPacket& operator >> (unsigned char&);

	CPacket& operator >> (short&);
	CPacket& operator >> (unsigned short&);

	CPacket& operator >> (int&);
	CPacket& operator >> (unsigned int&);

	CPacket& operator >> (long&);
	CPacket& operator >> (unsigned long&);

	CPacket& operator >> (__int64&);
	CPacket& operator >> (unsigned __int64&);

	CPacket& operator >> (float&);
	CPacket& operator >> (double&);

	//protected:
	char* pPush;
	char* pPop;
	char* buffer;
	char* checksum;
	int refCount;

	int	iBufferSize;
	int headerSize;
	static CBucketPool<CPacket> pool;
	long encodeFlag; // Change

	int SetHeader(char* dest, int pos, int size);
	int GetHeader(char* dest, int pos, int size);
	int Encoding();
	int Decoding();

	static CPacket* Alloc()
	{
		CPacket* ptr;

		ptr = pool.Alloc();
		if (ptr == nullptr)
		{
			wprintf(L"Alloc Error\n");
			return nullptr;
		}
		if (ptr->GetRefCount() != 0)
		{
			DebugBreak();
		}
		ptr->Clear();
		return ptr;
	}

	static void Free(CPacket* packet)
	{
		// CBucketPool<Node>
		pool.Free(packet);
	}

	static int GetUseBucket()
	{
		return pool.GetUseBucket();
	}
	static int GetTotalBucket()
	{
		return pool.GetTotalBucket();
	}
	static int GetTotalNode()
	{
		return pool.GetTotalNode();
	}

	static int GetUseNode()
	{
		return pool.GetUseNode();
	}

	long GetRefCount()
	{
		return refCount;
	}

	long AddRef()
	{
		return InterlockedIncrement((long*)&refCount);
	}


	long subRef()
	{
		long nowCount;
		nowCount = InterlockedDecrement((long*)&refCount);
		if (nowCount == 0)
		{
			CPacket::Free(this);
		}
		else if (nowCount < 0)
		{
			DebugBreak();
		}
		return nowCount;
	}


	char* GetCheckPtr()
	{
		return checksum;
	}
	int GetTotal()
	{
		int total = 0;
		char* s = checksum + 1;
		while (s != pPush)
		{
			total += *s;
			s++;
		}
		return total;
	}

};

#endif