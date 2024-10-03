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
            // TODO. ���⸦ �� �����ؾ��Ѵ�
            //���⼭ nullptr�� �Ұ���
            //�޸𸮸� �� �ø������� ���ؾ��Ѵ�.
            //�ø��� -> � ��å���� �÷����ϳ�?
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
