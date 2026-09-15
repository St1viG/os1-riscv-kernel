#include "../h/scheduler.hpp"

ThreadQueue Scheduler::ready;
_thread* Scheduler::idle = nullptr;


void Scheduler::put(_thread *t){
    if(!t || t == idle)
        return;
    ready.put(t);
}

_thread* Scheduler::get(){
    _thread* t = ready.get();
    return t ? t : idle;
}
