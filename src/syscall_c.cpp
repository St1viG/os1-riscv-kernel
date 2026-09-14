#include "../h/syscall_c.hpp"
#include "../lib/console.h"

static inline uint64 syscall1(uint64 code){
    register uint64 a0 __asm__("a0") = code;
    __asm__ volatile ("ecall" : "+r"(a0) : : "memory");
    return a0;
}

static inline uint64 syscall2(uint64 code, uint64 arg1){
    register uint64 a0 __asm__("a0") = code;
    register uint64 a1 __asm__("a1") = arg1;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return a0;
}

void* mem_alloc(size_t size){
    if(size==0) return nullptr;
    if(size > (size_t)-1 - (MEM_BLOCK_SIZE - 1)) return nullptr;
    size_t blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return (void*)syscall2(0x01, blocks);
}

int mem_free(void* ptr){
    return (int)syscall2(0x02, (uint64)ptr);
}


char getc(){ return __getc();}
void putc(char c){ __putc(c);}