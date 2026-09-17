#include "../h/MemoryAllocator.hpp"

const size_t MemoryAllocator::MAGIC;

MemoryAllocator& MemoryAllocator::getInstance(){
    static MemoryAllocator instance;
    return instance;
}

MemoryAllocator::MemoryAllocator(){
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
        DataBlock* rest = (DataBlock*)((char*)contender + need * MEM_BLOCK_SIZE);
        rest->size = remaining;
        rest->magic = 0;
        rest->prev = contender->prev;
        rest->next = contender->next;
        if(rest->prev) rest->prev->next = rest; else freeList = rest;
        if(rest->next) rest->next->prev = rest;

        contender->size = need;     // so mem_free returns exactly what was handed out
    }else{
        if(contender->prev) contender->prev->next = contender->next; else freeList = contender->next;
        if(contender->next) contender->next->prev = contender->prev;
    }

    contender->next = nullptr;
    contender->prev = nullptr;
    contender->magic = MAGIC;

    return (char*)contender + MEM_BLOCK_SIZE;
}

int MemoryAllocator::mem_free(void* ptr){
    if(ptr == nullptr)
        return -1;

    DataBlock* seg = (DataBlock*)((char*)ptr - MEM_BLOCK_SIZE);

    if((size_t)seg % MEM_BLOCK_SIZE != 0 || (size_t)seg < heapStart || (size_t)seg >= heapEnd)
        return -2;

    if(seg->magic != MAGIC)
        return -2;

    // Written as a division so a garbage size cannot overflow the multiply.
    if(seg->size == 0 || seg->size > (heapEnd - (size_t)seg) / MEM_BLOCK_SIZE)
        return -2;

    seg->magic = 0;

    DataBlock* prev = nullptr;
    DataBlock* next = freeList;
    while(next != nullptr && next < seg){
        prev = next;
        next = next->next;
    }

    seg->prev = prev;
    seg->next = next;
    if(prev) prev->next = seg; else freeList = seg;
    if(next) next->prev = seg;

    if(next != nullptr && (char*)seg + seg->size * MEM_BLOCK_SIZE == (char*)next){
        seg->size += next->size;
        seg->next = next->next;
        if(next->next) next->next->prev = seg;
    }

    if(prev != nullptr && (char*)prev + prev->size * MEM_BLOCK_SIZE == (char*)seg){
        prev->size += seg->size;
        prev->next = seg->next;
        if(seg->next) seg->next->prev = prev;
    }

    return 0;
}
