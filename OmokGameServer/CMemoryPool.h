#ifndef  __CMEMORY_POOL__
#define  __CMEMORY_POOL__

#include <new.h>
#include <iostream>
#include <Windows.h>

#define USER_MODE 0x00007fffffffffff
#define STAMP_MODE 0xffff800000000000

template <class T>
class CMemoryPool
{
private:
	/*
	* 블록 구조체
	*/
	struct st_BLOCK_NODE
	{
		template<typename... Args>
		st_BLOCK_NODE(Args&&... args)
			:
#ifdef DEBUG
			m_lowerBlock(nullptr),
			m_upperBlock(nullptr),
#endif
			m_data(std::forward<Args>(args)...),
			m_next(nullptr)
		{ }

		// st_BLOCK_NODE's member variable
#ifdef DEBUG
		st_BLOCK_NODE* m_lowerBlock;
#endif
		T				m_data;
		st_BLOCK_NODE* m_next;
#ifdef DEBUG
		st_BLOCK_NODE* m_upperBlock;
#endif
	};

public:
	/*
	* 생성자
	* parameters: (int) 초기 블럭 개수.
	*			  (bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	* return:     (void)
	*/
	template<typename... Args>
	CMemoryPool(int blockNum, bool placementNew = false, Args&&... args)
		: m_block_size(sizeof(st_BLOCK_NODE)),
		m_alloc_count(blockNum),
		m_use_count(0),
		m_placement_new_flag(placementNew)
		//m_stamp(0)
	{
		st_BLOCK_NODE* prevNode = nullptr;
		st_BLOCK_NODE* newNode = nullptr;


		// for index
		int i;

		if (m_placement_new_flag == false)
		{
			for (i = 0; i < blockNum; ++i)
			{
				newNode = new st_BLOCK_NODE(std::forward<Args>(args)...);
				newNode->m_next = prevNode;
				prevNode = newNode;
			}
		}
		else
		{
			/*for (i = 0; i < blockNum; ++i)
			{
				newNode = (st_BLOCK_NODE*)malloc(m_block_size);
				newNode->m_next = prevNode;
				prevNode = newNode;
			}*/

			for (i = 0; i < blockNum; ++i)
			{
				void* blockMemory = malloc(m_block_size);
				newNode = new (blockMemory) st_BLOCK_NODE(std::forward<Args>(args)...);  // placement new
				newNode->m_next = prevNode;
				prevNode = newNode;
			}
		}
		m_pFreeNode = newNode;
	}

	/*
	* 파괴자
	* parameters: (void)
	* return:     (void)
	*/
	virtual	~CMemoryPool()
	{
		// TODO: m_pFreeNode 풀 해제 코드 만들어야 할 듯. m_placement_new_flag를 통해서 나누면 될 듯
		if (m_pFreeNode == nullptr)
			int a = 0;
		else
			int b = 0;
		printf("CMemoryPool Free\n");
	}

	/*
	* 블럭 하나를 할당받는다.
	* parameters: (void)
	* return: (T *: 데이타 블럭 포인터)
	*/
	T* Alloc(void) // Pop
	{
		st_BLOCK_NODE* prev_top;
		st_BLOCK_NODE* real_top;
		st_BLOCK_NODE* next_node;

		st_BLOCK_NODE* tmp_node;

		// new
		T* new_node;

		PVOID prev_ptr;

		// stamp
		uintptr_t now_stamp;
		long now_alloc_count = m_alloc_count;
		long now_use_count = InterlockedIncrement(&m_use_count);

		if (now_alloc_count >= now_use_count)
		{
			do {
				prev_top = m_pFreeNode;
				real_top = (st_BLOCK_NODE*)((uintptr_t)prev_top & USER_MODE);
				now_stamp = ((uintptr_t)prev_top & STAMP_MODE);
				next_node = real_top->m_next;
			} while ((prev_ptr = InterlockedCompareExchangePointer(
				(PVOID*)&m_pFreeNode,
				(PVOID)((uintptr_t)next_node | now_stamp),
				prev_top)) != prev_top);

			if (m_placement_new_flag == true)
			{
				// excute placement new
#ifdef DEBUG
				new_node = new ((char*)real_top + sizeof(void*)) T;
#else
				new_node = new ((char*)real_top) T;
#endif
			}
			else
				new_node = (T*)real_top;
		}
		else
		{
			if (m_placement_new_flag == false)
			{
				tmp_node = new st_BLOCK_NODE;
#ifdef DEBUG
				new_node = (T*)((char*)tmp_node + sizeof(void*));
#else
				new_node = (T*)tmp_node;
#endif
			}
			else
			{
				tmp_node = (st_BLOCK_NODE*)malloc(m_block_size);
				tmp_node->m_next = nullptr;

#ifdef DEBUG
				new_node = new ((char*)tmp_node + sizeof(void*)) T;
#else
				new_node = new ((char*)tmp_node) T;
#endif

				//return tmp_node;
			}
			InterlockedIncrement(&m_alloc_count);
		}

		return new_node;
	}

	/*
	* 사용중이던 블럭을 해제한다.
	* parameters: (DATA *) 블럭 포인터.
	* return: (bool) TRUE, FALSE.
	*/
	bool	Free(T* pData) //PUSH
	{
		st_BLOCK_NODE* pBlockNode;
		st_BLOCK_NODE* prev_top;
		st_BLOCK_NODE* real_top;

		uintptr_t now_stamp;
		//uintptr_t nextStamp;
		now_stamp = InterlockedIncrement(&m_stamp);

#ifdef DEBUG
		pBlockNode = (st_BLOCK_NODE*)((char*)pData - sizeof(void*));
#else
		pBlockNode = (st_BLOCK_NODE*)pData;
#endif

		do {
			prev_top = m_pFreeNode;
			real_top = (st_BLOCK_NODE*)((uintptr_t)prev_top & USER_MODE);
			pBlockNode->m_next = real_top;
		} while (InterlockedCompareExchangePointer(
			(PVOID*)&m_pFreeNode,
			(PVOID)((uintptr_t)pBlockNode | (now_stamp << 47)),
			prev_top) != prev_top);

		/*do {
			prev_top = m_pFreeNode;
			real_top = (st_BLOCK_NODE*)((uintptr_t)prev_top & USER_MODE);
			nextStamp = ((uintptr_t)prev_top >> 47) + 1;
			pBlockNode->m_next = real_top;
		} while (InterlockedCompareExchangePointer(
			(PVOID*)&m_pFreeNode,
			(PVOID)((uintptr_t)pBlockNode | (nextStamp << 47)),
			prev_top) != prev_top);*/

		if (m_placement_new_flag == true)
		{
			pData->~T();
		}

		InterlockedDecrement(&m_use_count);

		return true;
	}

	/*
	* 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	* parameters: (void)
	* return:	  (int:  메모리 풀 내부 전체 개수)
	*/
	int		GetAllocCount(void)
	{
		return m_alloc_count;
	}

	/*
	* 현재 사용 중인 블럭 개수를 얻는 함수
	* parameters: (void)
	* return:	  (int: 사용중인 블럭 개수)
	*/
	int		GetUseCount(void)
	{
		return m_use_count;
	}

	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.

private: // member variable
	unsigned long		m_block_size;
	long				m_alloc_count;
	long				m_use_count;
	bool				m_placement_new_flag;
	st_BLOCK_NODE* m_pFreeNode;

	// stamp
	uintptr_t			m_stamp;
};

#endif
