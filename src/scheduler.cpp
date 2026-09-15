#include "../h/scheduler.hpp"

_thread* Scheduler::head = nullptr;
_thread* Scheduler::tail = nullptr;
_thread* Scheduler::idle = nullptr;


void Scheduler::put(_thread *t){
    if(!t || t == idle)
        return;
    t->next = nullptr;
    if(tail){
        tail->next = t;
        tail = t;
    }else{
        head = tail = t;
    }
}

_thread* Scheduler::get(){
    if(!head)
        return idle;
    _thread* t = head;
    head = head->next;
    if(!head)
        tail = nullptr;
    t->next = nullptr;
    return t;
}