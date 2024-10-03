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
        if (freeIndex < freeBlocks.size()) {
            return freeBlocks[freeIndex++];
        }
        else {
            // TODO. 여기를 더 수정해야한다
            //여기서 nullptr을 할건지
            //메모리를 더 늘릴건지를 정해야한다.
            //늘리자 -> 어떤 정책으로 늘려야하나?
            //1. 
           // pool.resize(blockSize * 1.5);

           // blockCount *= 1.5;

            return nullptr; // No more blocks available
        }
    }

    void deallocate(ExpOver* ptr) {
        std::lock_guard<std::mutex> lock(mutex); 
        if (ptr >= pool.data() && ptr < pool.data() + pool.size()) {
            freeBlocks[--freeIndex] = ptr;
        }
    }

    void TestPrint(ExpOver* ptr)
    {
        printf("%d %d\n", ptr->obj_id, ptr->target_id);
    }

};
