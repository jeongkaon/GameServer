#pragma once


class TimerEvent;
class ExpOver;

class MemoryPool {
    size_t blockSize;                      
    size_t blockCount;                     
    std::vector<ExpOver> pool;
    std::vector<ExpOver*> freeBlocks;

    size_t freeIndex;                      
    //std::atomic<size_t> freeIndex;
    std::mutex mutex; 

public:
    MemoryPool()
        : blockSize(256), blockCount(100), pool(blockCount* blockSize), freeIndex(0) {
        for (size_t i = 0; i < blockCount; ++i) {
            freeBlocks.push_back(&pool[i * blockSize]);
        }
    }

 
    MemoryPool(size_t blockSize, size_t blockCount)
        : blockSize(blockSize), blockCount(blockCount), pool(blockCount* blockSize), freeIndex(0) {
        for (size_t i = 0; i < blockCount; ++i) {
            freeBlocks.push_back(&pool[i * blockSize]);
        }
    }

    ExpOver* allocate() {
        std::lock_guard<std::mutex> lock(mutex); 
        if (!(freeIndex < freeBlocks.size())) {
            pool.resize(blockCount * blockSize * 1.5);
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

class MemoryPool1 {
    size_t blockSize;
    size_t blockCount;
    std::vector<ExpOver> pool;
    std::vector<ExpOver*> freeBlocks;

    std::atomic<size_t> freeIndex;
    std::mutex mutex;

public:


    MemoryPool1(size_t blockSize, size_t blockCount)
        : blockSize(blockSize), blockCount(blockCount), pool(blockCount* blockSize), freeIndex(0) {
        for (size_t i = 0; i < blockCount; ++i) {
            freeBlocks.push_back(&pool[i * blockSize]);
        }
    }

    ExpOver* allocate() {
        size_t index = freeIndex.load(std::memory_order_acquire);
        while (index < freeBlocks.size()) {
            if (freeIndex.compare_exchange_weak(index, index + 1, std::memory_order_acquire)) {
                return freeBlocks[index];
            }
        }

        // Resize the pool if no free blocks are available
        size_t newBlockCount = static_cast<size_t>(blockCount * 1.5);
        pool.resize(newBlockCount * blockSize);
        for (size_t i = blockCount; i < newBlockCount; ++i) {
            freeBlocks.push_back(&pool[i * blockSize]);
        }
        blockCount = newBlockCount;

        // Try to allocate again
        index = freeIndex.load(std::memory_order_acquire);
        freeIndex.compare_exchange_strong(index, index + 1, std::memory_order_acquire);
        return freeBlocks[index];
    }

    void deallocate(ExpOver* ptr) {
        size_t index = freeIndex.load(std::memory_order_relaxed);
        if (ptr >= pool.data() && ptr < pool.data() + pool.size()) {
            freeBlocks[--index] = ptr;
            freeIndex.store(index, std::memory_order_release);
        }
    }


};
