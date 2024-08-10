#pragma once
#pragma comment(lib, "Winmm.lib")

#include <Windows.h>

#include "CMemoryPool.h"
#define USER_MODE 0x00007fffffffffff

template <typename T>
class CLFQueue
{
private:
    alignas (64) long size;

    struct Node

    {
        Node()
            : next(nullptr)
        {}
        T data;
        Node* next;
    };

    Node* head;        // 시작노드를 포인트한다.
    alignas (64) Node* tail;        // 마지막노드를 포인트한다.
    CMemoryPool<Node> _node_pool{201};

public:
    CLFQueue()
        : size(0)
    {
        //head = new Node;
        head = _node_pool.Alloc();
        tail = head;
    }
    CLFQueue(int jobCount)
        : size(0), _node_pool{ jobCount }
    {
        //head = new Node; // 문제점.
        head = _node_pool.Alloc();
        tail = head;
    }
    int GetSize()
    {
        return size;
        //return InterlockedOr(&size, 0);
    }
    int GetPoolAllocSize()
    {
        return _node_pool.GetAllocCount();
    }
    int GetPoolUseSize()
    {
        return _node_pool.GetUseCount();
    }
    void Enqueue(T t)
    {
        Node* newNode;
        Node* prevTail;
        Node* realTail;
        Node* prevNext;
        uintptr_t tailStamp;
        uintptr_t myIndex;
        PVOID ret;
    // -----

        newNode = _node_pool.Alloc();
        newNode->next = nullptr;
        newNode->data = t;

        while (true)
        {
            prevTail = tail;
            realTail = (Node*)((uintptr_t)prevTail & USER_MODE);
            tailStamp = ((uintptr_t)prevTail >> 47) + 1;

            prevNext = realTail->next;

            if (prevNext == nullptr)
            {
                if ((ret = InterlockedCompareExchangePointer((PVOID*)&realTail->next, newNode, prevNext)) == prevNext)
                {
                    ret = InterlockedCompareExchangePointer((PVOID*)&tail, (PVOID)((uintptr_t)newNode | (tailStamp << 47)), prevTail);
                    break;
                }
            }
            else
            {
                // 멈춰있던 Cas1이 성공해버림.
                // prevNext에 값을 빼야함.
                ret = InterlockedCompareExchangePointer((PVOID*)&tail, (PVOID)((uintptr_t)prevNext | (tailStamp << 47)), prevTail);
            }

        }
        InterlockedExchangeAdd(&size, 1); // TODO
    }

    int Dequeue(T& t)
    {
        Node* prevHead;
        Node* realHead;
        Node* prevHeadNext;

        Node* prevTail;
        Node* realTail;
        Node* prevTailNext;

        uintptr_t headStamp;
        uintptr_t tailStamp;
        uintptr_t myIndex;
        PVOID ret;
        long retSize;

        retSize = InterlockedDecrement(&size);
        if (retSize < 0)
        {
            InterlockedIncrement(&size);
            return -1;
        }

        while (true)
        {
            prevHead = head;
            headStamp = ((uintptr_t)prevHead >> 47) + 1;
            realHead = (Node*)((uintptr_t)prevHead & USER_MODE);
            prevHeadNext = realHead->next;

            prevTail = tail;
            tailStamp = ((uintptr_t)prevTail >> 47) + 1;
            realTail = (Node*)((uintptr_t)prevTail & USER_MODE);
            prevTailNext = realTail->next;

            if (prevHeadNext == NULL)
            {
                continue;
            }
            else
            {
                /*if (prevTailNext != nullptr)
                {
                    InterlockedCompareExchangePointer((PVOID*)&tail, (PVOID)((uintptr_t)prevTailNext | (tailStamp << 47)), prevTail);
                }*/
                if (prevHead == tail)
                {
                    ret = InterlockedCompareExchangePointer((PVOID*)&tail, (PVOID)((uintptr_t)prevTailNext | (tailStamp << 47)), prevTail);
                }
                t = prevHeadNext->data;
                head = (Node *)((uintptr_t)prevHeadNext | (headStamp << 47));
                _node_pool.Free(realHead);
                break;
                /*if ((ret = InterlockedCompareExchangePointer((PVOID*)&head, (PVOID)((uintptr_t)prevHeadNext | (headStamp << 47)), prevHead)) == prevHead)
                {
                    _node_pool.Free(realHead);
                    break;
                }*/
            }
        }

        return 0;
    }
};