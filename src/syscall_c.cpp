#include "../h/syscall_c.hpp"

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

static inline uint64 syscall3(uint64 code, uint64 arg1, uint64 arg2){
    register uint64 a0 __asm__("a0") = code;
    register uint64 a1 __asm__("a1") = arg1;
    register uint64 a2 __asm__("a2") = arg2;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2) : "memory" );
    return a0;
}

static inline uint64 syscall5(uint64 code, uint64 arg1, uint64 arg2,
                              uint64 arg3, uint64 arg4){
    register uint64 a0 __asm__("a0") = code;
    register uint64 a1 __asm__("a1") = arg1;
    register uint64 a2 __asm__("a2") = arg2;
    register uint64 a3 __asm__("a3") = arg3;
    register uint64 a4 __asm__("a4") = arg4;
    __asm__ volatile ("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "memory");
    return a0;
}

void* mem_alloc(size_t size){
    if (size == 0) return nullptr;
    if (size > (size_t)-1 - (MEM_BLOCK_SIZE - 1)) return nullptr;   // rounding would wrap
    size_t blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;   // bytes -> blocks
    return (void*)syscall2(0x01, blocks);
}

int  mem_free(void* ptr)                    { return (int)syscall2(0x02, (uint64)ptr); }

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg){
    void* stack = mem_alloc(DEFAULT_STACK_SIZE);
    if(!stack) return -1;
    void* stack_space = (char*)stack + DEFAULT_STACK_SIZE;
    int res =  (int)syscall5(0x11, (uint64)handle, (uint64)start_routine, (uint64)arg, (uint64)stack_space);
    if (res != 0) mem_free(stack);
    return res;
}

int  thread_exit()                          { return (int)syscall1(0x12); }
void thread_dispatch()                      { syscall1(0x13); }
void thread_barrier() { syscall1(0x14); }

int  sem_open(sem_t* handle, unsigned init) { return (int)syscall3(0x21, (uint64)handle, init); }
int  sem_close(sem_t handle)                { return (int)syscall2(0x22, (uint64)handle); }
int  sem_wait(sem_t id)                     { return (int)syscall2(0x23, (uint64)id); }
int  sem_signal(sem_t id)                   { return (int)syscall2(0x24, (uint64)id); }
int  sem_wait_n(sem_t id, unsigned n)       { return (int)syscall3(0x25, (uint64)id, n); }
int  sem_signal_n(sem_t id, unsigned n)     { return (int)syscall3(0x26, (uint64)id, n); }

int  time_sleep(time_t t)                   { return (int)syscall2(0x31, t); }

char getc()                                 { return (char)syscall1(0x41); }
void putc(char c)                           { syscall2(0x42, (uint64)c); }
