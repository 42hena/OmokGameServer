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
		// ����.�ʿ�.
		/*if (_freeList == nullptr)
		{

		}*/

		SPoolNode* pNode;
		pNode = _freeList;
		_freeList = _freeList->_pNext;	// ���� ������ ����Ŵ.

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

	SPoolNode* _pFreeList; // nullptr�̾�� ��.

	int _totalCount;
	int _useCount;

	// SRWLock;
};



// �޸� Ǯ.
// stack ����

// �� �����.
// ����� ����, ���� ����.
// �� �ʿ�.


