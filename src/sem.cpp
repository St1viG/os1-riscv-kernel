#include "../h/sem.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/scheduler.hpp"

void* _sem::operator new(size_t bytes){
    size_t blocks = (bytes + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return MemoryAllocator::getInstance().mem_alloc(blocks);
}

void _sem::operator delete(void* ptr) noexcept{
    MemoryAllocator::getInstance().mem_free(ptr);
}

_sem::_sem(unsigned init) : magic(MAGIC), value(init), blocked() {}

int _sem::open(sem_t* handle, unsigned init){
    if(!handle)
        return ERR_HANDLE;

    // Allocate, test, then construct in place. The throwing operator new is the
    // form the compiler may assume never returns null, so it emits no check.
    // Same dance as _thread::createThread, same reason.
    void* raw = _sem::operator new(sizeof(_sem));
    if(!raw)
        return ERR_HANDLE;

    *handle = new (raw) _sem(init);
    return OK;
}

int _sem::close(sem_t handle){
    if(!handle || !handle->valid())
        return ERR_HANDLE;
    
    handle->releaseAll();

    // Volatile on purpose. A plain `handle->magic = 0` here is a dead store by
    // the letter of the standard -- the object's lifetime ends on the next
    // line, so nothing may legitimately read it again -- and g++ -Og duly
    // deletes it, leaving MAGIC intact in the freed block and every stale
    // handle passing valid(). Catching the code that reads it anyway is the
    // whole point of the guard, so the write has to be made unremovable.
    *(volatile uint64*)&handle->magic = 0;

    delete handle;
    return OK;
}

int _sem::wait(sem_t handle, unsigned n){
    if(!handle || !handle->valid())
        return ERR_HANDLE;
    if(n == 0)
        return OK;
    
    if(handle->blocked.isEmpty() && handle->value >= n){
        handle->value -= n;
        return OK;
    }

    _thread* caller = _thread::running;
    caller->semRequest = n;
    caller->semResult = OK;
    caller->setBlocked(true);
    handle->blocked.put(caller);

    // The kernel call, NOT the C API thread_dispatch(): we are already inside
    // the trap handler, and an ecall here would nest a trap and clobber sepc.
    _thread::dispatch();

    // Resumed. `handle` may be dangling -- close() frees the object and only
    // then releases its waiters, so the result lives in our own TCB.
    return caller->semResult;
}

int _sem::signal(sem_t handle, unsigned n){
    if(!handle || !handle->valid())
        return ERR_HANDLE;
    if(n==0)
        return OK;
    if(handle->value > (unsigned)~0u - n)
        return ERR_OVERFLOW;
    
    handle->value += n;
    handle->releaseReady();
    return OK;
}

void _sem::releaseReady(){
    while(!blocked.isEmpty() && blocked.peek()->semRequest <= value){
        _thread* t = blocked.get();
        value -= t->semRequest;
        t->semResult = OK;
        t->setBlocked(false);
        Scheduler::put(t);
    }
}

void _sem::releaseAll(){
    while(!blocked.isEmpty()){
        _thread* t = blocked.get();
        t->semResult = ERR_CLOSED;
        t->setBlocked(false);
        Scheduler::put(t);
    }
}