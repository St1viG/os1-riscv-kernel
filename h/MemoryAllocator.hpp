#ifndef __MEMORYALLOCATOR_H__
#define __MEMORYALLOCATOR_H__

#include "../lib/hw.h"

// Continuous allocator over [HEAP_START_ADDR, HEAP_END_ADDR).
//
// Every segment starts on a MEM_BLOCK_SIZE boundary and spans a whole number of
// blocks. Its first block holds the header, the rest is payload -- so the
// pointer handed to the caller is block-aligned too. Free segments are kept in
// one doubly-linked list sorted by address, which is what makes coalescing in
// mem_free a matter of checking two neighbours.
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

    // Stamped into the header on alloc, cleared on free, so mem_free can reject
    // a pointer that did not come from mem_alloc instead of corrupting the list.
    static const size_t MAGIC = 0xDEADBEEFA110C8EDUL;

    MemoryAllocator();
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    DataBlock* freeList;            // sorted by address
    size_t heapStart;
    size_t heapEnd;
};

#endif // __MEMORYALLOCATOR_H__
