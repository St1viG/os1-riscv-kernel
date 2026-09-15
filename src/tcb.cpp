#include "../h/tcb.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/riscv.hpp"
#include "../h/scheduler.hpp"


_thread* _thread::running = nullptr;
_thread* _thread::zombie = nullptr;
_thread* _thread::sleepHead = nullptr;


void* _thread::operator new(size_t bytes){
    size_t blocks = (bytes + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return MemoryAllocator::getInstance().mem_alloc(blocks);
}

void _thread::operator delete(void* ptr) noexcept{
    MemoryAllocator::getInstance().mem_free(ptr);
}


_thread::_thread(Body body, void* arg, void* stackBase, void* stackSpace, bool userMode) : context({body ? (uint64)&threadWrapper : 0, body ? (uint64)stackSpace : 0 }), body(body), arg(arg), stackBase(stackBase), userMode(userMode), finished(false), blocked(false), semRequest(0), semResult(0), timeSlice(DEFAULT_TIME_SLICE), sleepTime(0), next(nullptr){}


_thread* _thread::createThread(Body body, void *arg, void *stackBase, void *stackSpace, bool userMode){
    void* raw = _thread::operator new(sizeof(_thread));
    if(!raw)
        return nullptr;
    return new (raw) _thread(body,arg,stackBase,stackSpace,userMode);
}



void _thread::threadWrapper(){
    reap();
    // Cached before the mode switch on purpose: past popSppSpie we are user
    // code, and `running` is a kernel pointer.
    Body b = running->body;
    void* a = running->arg;
    bool user = running->userMode;

    // First exit from the kernel. Both branches SET the state rather than
    // inherit it -- we run on whatever sstatus the PREVIOUS thread's trap left
    // behind, and once the timer can preempt, that thread may have been in
    // either mode. Inheriting SPP=1 from a kernel thread starts a user thread
    // in supervisor mode, which only test 7 would ever notice.
    if(user){
        Riscv::mc_sstatus(Riscv::SSTATUS_SPP);      // sret -> user mode
        Riscv::ms_sstatus(Riscv::SSTATUS_SPIE);     // sret -> SIE = 1
        Riscv::popSppSpie();
    }else{
        // Kernel threads never sret, so nothing would otherwise re-enable SIE
        // for them: they are entered from inside a trap, where it is already 0.
        // The idle thread is one of these, and an idle thread that cannot be
        // interrupted means a fully-asleep system never wakes up again.
        Riscv::ms_sstatus(Riscv::SSTATUS_SIE);
    }

    b(a);
    thread_exit();
}

void _thread::dispatch(){
    _thread* old = running;
    if(!old->isFinished() && !old->isBlocked())
        Scheduler::put(old);
    running = Scheduler::get();
    // Every thread is switched in with a full quantum, however it got here:
    // preempted, yielding, or unblocking. A thread that yields must not be
    // charged the remainder of someone else's slice.
    running->timeSlice = DEFAULT_TIME_SLICE;
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

void _thread::tick(){
    // Sleepers first: anything coming due this tick must already be on the ready
    // queue before the preemption below picks a successor.
    wakeSleepers();

    if(running->timeSlice > 0)
        running->timeSlice--;
    if(running->timeSlice == 0)
        dispatch();          // dispatch() hands the next thread a fresh quantum
}

void _thread::wakeSleepers(){
    // Only the head is decremented -- every gap behind it is already relative to
    // it, so the whole list ages in one subtraction.
    if(sleepHead && sleepHead->sleepTime > 0)
        sleepHead->sleepTime--;

    // Several can come due together: a zero gap to the head means the same tick.
    while(sleepHead && sleepHead->sleepTime == 0){
        _thread* t = sleepHead;
        sleepHead = t->next;
        t->next = nullptr;
        t->setBlocked(false);
        Scheduler::put(t);
    }
}

int _thread::sleep(time_t ticks){
    if(ticks == 0)
        return 0;

    _thread* t = running;
    t->sleepTime = ticks;    // absolute for now; the walk converts it to a gap
    t->setBlocked(true);     // stops dispatch() re-queueing us as ready

    // Insert into the delta list. `cur` points TO the link that may need
    // rewriting, so inserting at the head and in the middle are the same three
    // lines. `<=` puts equal deadlines behind the ones already there, so
    // same-tick sleepers wake in FIFO order.
    _thread** cur = &sleepHead;
    while(*cur && (*cur)->sleepTime <= t->sleepTime){
        t->sleepTime -= (*cur)->sleepTime;      // subtract off what precedes us
        cur = &(*cur)->next;
    }
    t->next = *cur;
    if(t->next)
        t->next->sleepTime -= t->sleepTime;     // successor's gap shrinks by ours
    *cur = t;

    // The kernel call, NOT the C API time_sleep(): we are already inside the
    // trap handler, and an ecall here would nest a trap and clobber sepc. Same
    // reason _sem::wait uses this one.
    dispatch();
    return 0;
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
