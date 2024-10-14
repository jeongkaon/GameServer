#pragma once
class ExpOver;

struct MemoryBlock {
    MemoryBlock* next;
    std::vector<ExpOver> pool;

    MemoryBlock(size_t blockCount) : next(nullptr), pool(blockCount) {}
};

class MemoryPool {
    size_t blockSize;                      
    size_t blockCount;      

    MemoryBlock* head;
    std::vector<ExpOver*> freeBlocks;

    size_t freeIndex;                      
    std::mutex mutex; 

    void ResizePool();

public:
    MemoryPool(size_t blockSize, size_t blockCount);

    ExpOver* allocate();

    void deallocate(ExpOver* ptr);
 };

