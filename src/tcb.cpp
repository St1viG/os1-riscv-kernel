#include "../h/tcb.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/riscv.hpp"
#include "../h/scheduler.hpp"


_thread* _thread::running = nullptr;


_thread* _thread::zombie = nullptr;



void* _thread::operator new(size_t bytes){
    size_t blocks = (bytes + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return MemoryAllocator::getInstance().mem_alloc(blocks);
}

void _thread::operator delete(void* ptr) noexcept{
    MemoryAllocator::getInstance().mem_free(ptr);
}


_thread::_thread(Body body, void* arg, void* stackBase, void* stackSpace, bool userMode) : context({body ? (uint64)&threadWrapper : 0, body ? (uint64)stackSpace : 0 }), body(body), arg(arg), stackBase(stackBase), userMode(userMode), finished(false), blocked(false), semRequest(0), semResult(0), timeSlice(DEFAULT_TIME_SLICE), next(nullptr){}


_thread* _thread::createThread(Body body, void *arg, void *stackBase, void *stackSpace, bool userMode){
    void* raw = _thread::operator new(sizeof(_thread));
    if(!raw)
        return nullptr;
    return new (raw) _thread(body,arg,stackBase,stackSpace,userMode);
}



void _thread::threadWrapper(){
    reap();
    Body b = running->body;
    void* a = running->arg;
    bool user = running->userMode;
    if(user)
        Riscv::popSppSpie();
    b(a);
    thread_exit();
}

void _thread::dispatch(){
    _thread* old = running;
    if(!old->isFinished() && !old->isBlocked())
        Scheduler::put(old);
    running = Scheduler::get();
    contextSwitch(&old->context, &running->context);
    reap();
}

void _thread::exit(){
    running->finished = true;
    zombie = running;
    dispatch();
}

void _thread::reap(){
    if (!zombie) return;
    if(zombie->stackBase) MemoryAllocator::getInstance().mem_free(zombie->stackBase);
    delete zombie;
    zombie = nullptr;
}


void ThreadQueue::put(_thread* t){
    if(!t)
        return;
    t->next = nullptr;
    if(tail)
        tail->next = t;
    else
        head = t;
    tail = t;
}

_thread* ThreadQueue::get(){
    _thread* t = head;
    if(!t)
        return nullptr;
    head = head->next;
    if(!head)
        tail = nullptr;
    t->next = nullptr;
    return t;
}
