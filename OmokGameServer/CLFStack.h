#pragma once

#include "CMemoryPool.h"

template <typename T>
class CStack
{
private:
	struct Node
	{
		Node()
			: next(nullptr)
		{}
		T data;
		Node* next;
	};

public:

	CStack(int jobCount)
		: size(0), nodePool{ jobCount }
	{ }
	
	int GetSize()
	{
		return size;
	}
	int GetPoolAllocSize()
	{
		return nodePool.GetAllocCount();
	}
	int GetUseSize()
	{
		return GetPoolAllocSize() - size;
	}
	int GetPoolUseSize()
	{
		return nodePool.GetUseCount();
	}

	void Push(T data)
	{
		Node* newNode;
		Node* prevTop;
		Node* realTop;
		uintptr_t nextStamp;
// -----

		newNode = nodePool.Alloc();
		newNode->data = data;

		do {
			prevTop = topNode;
			realTop = (Node*)((uintptr_t)prevTop & 0x0000'7fff'ffff'ffff);
			nextStamp = (uintptr_t)prevTop >> 47 + 1;
			newNode->next = realTop;
		} while (InterlockedCompareExchangePointer((PVOID*)&topNode, (PVOID)((uintptr_t)newNode | nextStamp << 47), prevTop) != prevTop);

		InterlockedIncrement(&size);

		/*uintptr_t prevTop;
		do {
			prevTop = topNode;
			realTop = prevTop & 0x0000'7fff'ffff'ffff);
			nextStamp = (uintptr_t)prevTop >> 50 + 1;
			(st_Node*)newNode->next = (st_Node*)prevTop;
		} while (InterlockedCompareExchangePointer((PVOID*)&topNode, (newNode | nextStamp << 50), prevTop) != prevTop);
		InterlockedCompareExchange64((uintptr_t*)&topNode, , );*/
	}

	bool Pop(T& data)
	{
		Node* prevTop;
		Node* realTop;
		Node* prevNext;
		uintptr_t nowStamp;

		long ret = InterlockedDecrement(&size);
		if (ret < 0)
		{
			InterlockedIncrement(&size);
			return false;
		}

		do {
			prevTop = topNode;
			realTop = (Node*)((uintptr_t)prevTop & 0x0000'7fff'ffff'ffff);
			nowStamp = (uintptr_t)prevTop & 0xffff'8000'0000'0000;
			prevNext = realTop->next;
			data = realTop->data;
		} while (InterlockedCompareExchangePointer((PVOID*)&topNode, (PVOID)((uintptr_t)prevNext | nowStamp), prevTop) != prevTop);

		//delete prevTop;
		nodePool.Free(realTop);

		return true;
	}
private:

	long size;
	Node* topNode;

	CMemoryPool<Node> nodePool;
};