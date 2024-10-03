#pragma once


class TimerEvent;
class ExpOver;

class MemoryPool {
    size_t blockSize;                      
    size_t blockCount;                     
    std::vector<ExpOver> pool;
    std::vector<ExpOver*> freeBlocks;

    size_t freeIndex;                      
    std::mutex mutex; 

public:

 
    MemoryPool(size_t blockSize, size_t blockCount)
        : blockSize(blockSize), blockCount(blockCount), pool(blockCount* blockSize), freeIndex(0) {
        for (size_t i = 0; i < blockCount; ++i) {
            freeBlocks.push_back(&pool[i * blockSize]);
        }
    }

    ExpOver* allocate() {
        std::lock_guard<std::mutex> lock(mutex); 
        
        if (!(freeIndex < freeBlocks.size())) {
            pool.resize(blockSize * 1.5);
            blockCount *= 1.5;
        }
        return freeBlocks[freeIndex++];

    }

    void deallocate(ExpOver* ptr) {
        std::lock_guard<std::mutex> lock(mutex); 
        if (ptr >= pool.data() && ptr < pool.data() + pool.size()) {
            freeBlocks[--freeIndex] = ptr;
        }
    }

 

};
