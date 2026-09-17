#ifndef __MEMORYALLOCATOR_H__
#define __MEMORYALLOCATOR_H__

#include "../lib/hw.h"

class MemoryAllocator{
public:
    static MemoryAllocator& getInstance();

    void* mem_alloc(size_t size);   // size in BLOCKS -- the C API layer converts from bytes
    int mem_free(void* ptr);

private:
    struct DataBlock{
        DataBlock* next;
        DataBlock* prev;
        size_t size;                // total blocks in this segment, header block included
        size_t magic;               // MAGIC while allocated, 0 while free
    };

    static const size_t MAGIC = 0xDEADBEEFA110C8EDUL;

    MemoryAllocator();
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    DataBlock* freeList;            // sorted by address
    size_t heapStart;
    size_t heapEnd;
};

#endif // __MEMORYALLOCATOR_H__
