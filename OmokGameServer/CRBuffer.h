#pragma once

#include <memory.h>

class CRBuffer {
public:
	// 생성자
	CRBuffer(void)
		: _bufferSize(4000)
	{
		_bufferStart = new char[_bufferSize];
		_rear = _front = _bufferStart;

		_bufferEnd = _bufferStart + _bufferSize;
	}
	
	CRBuffer(int size)
		: _bufferSize(size)
	{
		_bufferStart = new char[_bufferSize];
		_rear = _front = _bufferStart;
		_bufferEnd = _bufferStart + _bufferSize;
	}

	// 소멸자
	~CRBuffer()
	{
		delete[] _bufferStart;
	}

	// Get total buffer size
	int		GetBufferSize(void)
	{
		return (_bufferSize);
	}

	int		GetUseSize(void)
	{
		char* prevFront;
		char* prevRear;

		prevRear = _rear;
		prevFront = _front;
		if (prevFront <= prevRear)
		{
			return (prevRear - prevFront);
		}
		else
		{
			return (prevRear + _bufferSize - prevFront);
		}
	}

	int		GetFreeSize(void)
	{
		return _bufferSize - 1 - GetUseSize();
	}

	int		DirectEnqueueSize(void)
	{
		char* prevFront;

		prevFront = _front;
		if (prevFront == _bufferStart)
		{
			return _bufferEnd - _rear - 1;
		}
		else
		{
			if (prevFront <= _rear)
			{
				return _bufferEnd - _rear;
			}
			else
				return prevFront - _rear - 1;
		}
	}

	int		DirectDequeueSize(void)
	{
		char* prevRear;

		prevRear = _rear;
		if (_front <= prevRear)
		{
			return prevRear - _front;
		}
		else
		{
			return _bufferEnd - _front;
		}
	}

	int		Enqueue(char* pData, int iSize)
	{
		int directEndSize;
		int freeSize;

		freeSize = GetFreeSize();
		if (iSize <= freeSize)
		{
			directEndSize = DirectEnqueueSize();
			if (directEndSize < iSize)
			{
				memcpy(_rear, pData, directEndSize);
				memcpy(_bufferStart, pData + directEndSize, iSize - directEndSize);
			}
			else
			{
				memcpy(_rear, pData, iSize);
			}
			int ret = MoveRear(iSize);

			if (ret != iSize)
			{
				DebugBreak();
			}

			return iSize;
		}
		else
			return 0;
	}

	int		Dequeue(char* pDest, int size)
	{
		int ret = Peek(pDest, size);
		if (ret != size)
		{
			DebugBreak();
		}
		int ret1 = MoveFront(ret);
		if (ret1 != ret)
		{
			DebugBreak();
		}
		return  ret1;
	}

	int		Peek(char* pDest, int size)
	{
		int directEndSize;
		if (GetUseSize() >= size)
		{
			directEndSize = DirectDequeueSize();
			if (directEndSize < size)
			{
				memcpy(pDest, _front, directEndSize);
				memcpy(pDest + directEndSize, _bufferStart, size - directEndSize);
			}
			else
			{
				memcpy(pDest, _front, size);
			}
			return size;
		}
		else
			return 0;
	}

	int		MoveRear(int size)
	{
		int remain;

		if (size <= GetFreeSize())
		{
			if (_rear + size < _bufferEnd) // <= 에서 < 로 교체
			{
				_rear += size;
			}
			else
			{
				remain = _rear + size - _bufferEnd;
				_rear = _bufferStart + remain;
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
			if (_front + size < _bufferEnd) // <= 에서 < 로 교체
			{
				_front += size;
			}
			else
			{
				remain = _front + size - _bufferEnd;
				_front = _bufferStart + remain;
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
		_rear = _front = _bufferStart;
	}

	char* GetBufferPtr(void)
	{
		return _bufferStart;
	}

	char* GetFrontBufferPtr(void)
	{
		return _front;
	}

	char* GetRearBufferPtr(void)
	{
		return _rear;
	}

private:
	char*	_bufferStart;
	char*	_bufferEnd;
	char*	_rear;
	char*	_front;
	int		_bufferSize;
};
