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

    // volatile: the object dies on the next line, so a plain store is dropped.
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

    _thread::dispatch();

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