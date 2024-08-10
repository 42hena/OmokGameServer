#pragma once
#include <memory.h>

//#define TEST 1

class CRBuffer {
public:
	// 생성자
	CRBuffer(void)
	{
		int size = 4000;
		bufferStart = new char[size];
		rear = front = bufferStart;
		bufferSize = size;
		bufferEnd = bufferStart + bufferSize;
	}
	
	CRBuffer(int size)
	{
		bufferStart = new char[size + 1];
		rear = front = bufferStart;
		bufferSize = size + 1;
		bufferEnd = bufferStart + bufferSize;
	}

	// 소멸자
	~CRBuffer()
	{
		delete[]bufferStart;
	}

	// Get total buffer size
	int		GetBufferSize(void)
	{
		return (bufferSize);
	}

	int		GetUseSize(void)
	{
		char* prevFront;
		char* prevRear;

		prevRear = rear;
		prevFront = front;
		if (prevFront <= prevRear)
		{
			return (prevRear - prevFront);
		}
		else
		{
			return (prevRear + bufferSize - prevFront);
		}
	}

	int		GetFreeSize(void)
	{
		return (bufferSize - 1 - GetUseSize());
	}

	int		DirectEnqueueSize(void)
	{
		char* prevFront;

		prevFront = front;
		if (prevFront == bufferStart)
		{
			return ((bufferEnd) - rear - 1);
		}
		else
		{
			if (prevFront <= rear)
			{
				return (bufferEnd - rear);
			}
			else
				return prevFront - rear - 1;
		}
	}

	int		DirectDequeueSize(void)
	{
		char* prevRear;

		prevRear = rear;
		if (front <= prevRear)
		{
			return prevRear - front;
		}
		else
		{
			return bufferEnd - front;
		}
	}

	int		Enqueue(char* chpData, int iSize)
	{
		int directEndSize;
		int freeSize;

		freeSize = GetFreeSize();
		if (iSize <= freeSize)
		{
			directEndSize = DirectEnqueueSize();
			if (directEndSize < iSize)
			{
				memcpy(rear, chpData, directEndSize);
				memcpy(bufferStart, chpData + directEndSize, iSize - directEndSize);
			}
			else
			{
				memcpy(rear, chpData, iSize);
			}
#ifdef TEST
			int remain;
			if (rear + iSize < bufferEnd) // <= 에서 < 로 교체
			{
				rear += iSize;
			}
			else
			{
				remain = rear + iSize - bufferEnd;
				rear = bufferStart + remain;
			}

			return iSize;
#endif
#ifndef TEST
			int ret = MoveRear(iSize);

			if (ret != iSize)
			{
				Sleep(0);
			}

			return (iSize);
#endif
		}
		else
			return (0);
	}

	int		Dequeue(char* chpDest, int iSize)
	{
#ifdef TEST
		int directEndSize;
		int useSize;

		useSize = GetUseSize();

		if (useSize >= iSize)
		{
			directEndSize = DirectDequeueSize();
			if (directEndSize < iSize)
			{
				memcpy(chpDest, front, directEndSize);
				memcpy(chpDest + directEndSize, bufferStart, iSize - directEndSize);
			}
			else
			{
				memcpy(chpDest, front, iSize);
			}
			if (front + iSize < bufferEnd) // <= 에서 < 로 교체
			{
				front += iSize;
			}
			else
			{
				int remain = front + iSize - bufferEnd;
				front = bufferStart + remain;
			}

			return (iSize);
		}
		else
			return (0);
#endif
#ifndef TEST
		int ret = Peek(chpDest, iSize);
		if (ret != iSize)
		{
			exit(0);
			Sleep(0);
		}
		int ret1 = MoveFront(ret);
		if (ret1 != ret)
		{
			exit(0);
			Sleep(0);
		}
		return  ret1;
#endif
	}

	int		Peek(char* chpDest, int iSize)
	{
		int directEndSize;
		if (GetUseSize() >= iSize)
		{
			directEndSize = DirectDequeueSize();
			if (directEndSize < iSize)
			{
				memcpy(chpDest, front, directEndSize);
				memcpy(chpDest + directEndSize, bufferStart, iSize - directEndSize);
			}
			else
			{
				memcpy(chpDest, front, iSize);
			}
			return (iSize);
		}
		else
			return (0);
	}

	int		MoveRear(int size)
	{
		int remain;

		if (size <= GetFreeSize())
		{
			if (rear + size < bufferEnd) // <= 에서 < 로 교체
			{
				rear += size;
			}
			else
			{
				remain = rear + size - bufferEnd;
				rear = bufferStart + remain;
			}
			return size;
		}
		else
		{
			return 0;
		}
	}

	int		MoveFront(int size)
	{
		int remain;

		if (size <= GetUseSize())
		{
			if (front + size < bufferEnd) // <= 에서 < 로 교체
			{
				front += size;
			}
			else
			{
				remain = front + size - bufferEnd;
				front = bufferStart + remain;
			}
			return size;
		}
		else
		{
			return 0;
		}
	}

	void	ClearBuffer(void)
	{
		rear = front = bufferStart;
	}

	char* GetBufferPtr(void)
	{
		return bufferStart;
	}

	char* GetFrontBufferPtr(void)
	{
		return front;
	}

	char* GetRearBufferPtr(void)
	{
		return rear;
	}

private:
	char* bufferStart;
	char* bufferEnd;
	char* rear;
	char* front;
	int		bufferSize;
};
