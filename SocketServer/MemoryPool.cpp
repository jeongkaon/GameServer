#include "stdafx.h"
#include "MemoryPool.h"
#include "ExpOver.h"

void MemoryPool::ResizePool()
{
    MemoryBlock* temp = head;
    MemoryBlock* newnode = new MemoryBlock(blockCount);
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = newnode;

    for (size_t i = 0; i < blockCount; ++i) {
        freeBlocks.emplace_back(&(newnode->pool[i]));
    }
}

MemoryPool::MemoryPool(size_t blockSize, size_t blockCount)
    : blockSize(blockSize), blockCount(blockCount),  freeIndex(0)
{
    head = new MemoryBlock(blockCount);
    for (size_t i = 0; i < blockCount; ++i) {
        freeBlocks.emplace_back(&(head->pool[i]));
    }
}

ExpOver* MemoryPool::allocate()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!(freeIndex < freeBlocks.size())) {
        ResizePool();
  
    }
    return freeBlocks[freeIndex++];
}

void MemoryPool::deallocate(ExpOver* ptr)
{
    std::lock_guard<std::mutex> lock(mutex);
    freeBlocks[--freeIndex] = ptr;
}
