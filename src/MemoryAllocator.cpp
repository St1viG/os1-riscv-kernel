#include "../h/MemoryAllocator.hpp"

const size_t MemoryAllocator::MAGIC;

MemoryAllocator& MemoryAllocator::getInstance(){
    static MemoryAllocator instance;
    return instance;
}

// Runs once, on the first getInstance() call. Lays the whole heap out as a
// single free segment.
MemoryAllocator::MemoryAllocator(){
    // Pull both ends inward to block boundaries -- hw.lib makes no alignment
    // promise about HEAP_START_ADDR, and every segment must start aligned.
    heapStart = ((size_t)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE;
    heapEnd = (size_t)HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE;

    // Fewer than two blocks leaves no room for a header plus any payload.
    if(heapEnd <= heapStart || (heapEnd - heapStart) / MEM_BLOCK_SIZE < 2){
        freeList = nullptr;
        return;
    }

    freeList = (DataBlock*)heapStart;
    freeList->next = nullptr;
    freeList->prev = nullptr;
    freeList->size = (heapEnd - heapStart) / MEM_BLOCK_SIZE;
    freeList->magic = 0;
}

void* MemoryAllocator::mem_alloc(size_t size){ // size = number of payload blocks requested
    if(size == 0)
        return nullptr;

    size_t need = size + 1;         // requested payload plus our header block
    if(need < size)                 // size == SIZE_MAX would wrap to 0
        return nullptr;

    // First fit.
    DataBlock* contender = nullptr;
    for(DataBlock* current = freeList; current != nullptr; current = current->next){
        if(current->size >= need){
            contender = current;
            break;
        }
    }
    if(contender == nullptr)
        return nullptr;

    size_t remaining = contender->size - need;
    if(remaining >= 2){
        // Enough left over for a header plus at least one payload block, so split.
        // `rest` takes over contender's exact position in the free list.
        DataBlock* rest = (DataBlock*)((char*)contender + need * MEM_BLOCK_SIZE);
        rest->size = remaining;
        rest->magic = 0;
        rest->prev = contender->prev;
        rest->next = contender->next;
        if(rest->prev) rest->prev->next = rest; else freeList = rest;
        if(rest->next) rest->next->prev = rest;

        contender->size = need;     // so mem_free returns exactly what was handed out
    }else{
        // A one-block remainder could only ever hold a header, so hand the whole
        // segment over rather than leaving an unusable stub in the list.
        if(contender->prev) contender->prev->next = contender->next; else freeList = contender->next;
        if(contender->next) contender->next->prev = contender->prev;
    }

    contender->next = nullptr;
    contender->prev = nullptr;
    contender->magic = MAGIC;

    return (char*)contender + MEM_BLOCK_SIZE;
}

int MemoryAllocator::mem_free(void* ptr){
    return 0;
}
