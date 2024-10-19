#pragma once
#include <iostream>

template <typename T>
class CPool
{
public:
	struct SPoolNode
		: pNext(nullptr)
	{
		void Init()
		{
			_pNext = nullptr;
		}

		T _data;
		SPoolNode* _pNext;
	};
	CPool()
	{
		using std::cout;
		cout << "CPool\n";
	}
	
	CPool(int defaultNodeCount)
	{
		SPoolNode* pLastNode;
		for (int i = 0; i < defaultNodeCount; ++i)
		{
			pLastNode = new SPoolNode;
			pLastNode->Init();
			pLastNode->pNext = _pFreeList;
			_pFreeList = pLastNode;
		}
	}
	
	~CPool()
	{
		using std::cout;
		cout << "~CPool\n";
	}

	/*
	CPool&(const CPool& copy) const;
	bool operator=(const CPool& copy);
	*/

	

	void* Alloc()
	{
		// 생성.필요.
		/*if (_freeList == nullptr)
		{

		}*/

		SPoolNode* pNode;
		pNode = _freeList;
		_freeList = _freeList->_pNext;	// 다음 스택을 가리킴.

		return pNode;
	}

	void Free(void* ptr)
	{
		SPoolNode* pNode = ptr;
		pNode->_pNext = _freeList;
		_freeList = pNode;
	}

	void GetTotalCount()
	{
		return _totalCount;
	}
	void GetUseCount()
	{
		return _useCount;
	}
	void GetFreeCount()
	{
		return GetTotalCount() - GetUseCount();
	}

private:

	SPoolNode* _pFreeList; // nullptr이어야 함.

	int _totalCount;
	int _useCount;

	// SRWLock;
};



// 메모리 풀.
// stack 형식

// 총 몇개인지.
// 사용중 개수, 나간 개수.
// 락 필요.


