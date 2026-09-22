#include "../h/syscall_cpp.hpp"

void* operator new(size_t n) {return mem_alloc(n);}
void operator delete(void* p) noexcept { mem_free(p);}
void* operator new[] (size_t n) { return mem_alloc(n);}
void operator delete[](void* p) noexcept { mem_free(p);}

Thread::Thread(void (*body)(void*), void* arg) : myHandle(nullptr), body(body), arg(arg){}
Thread::Thread() : myHandle(nullptr), body(nullptr), arg(nullptr) {}

int Thread::start(){
    if(myHandle) return -1;
    return thread_create(&myHandle, &Thread::wrapper, this);
}

void Thread::wrapper(void* arg) {
    Thread* t = static_cast<Thread*>(arg);
    if(t->body) t->body(t->arg);
    else    t->run();
}

Thread::~Thread() {} // to implement
void Thread::dispatch() { thread_dispatch();}
int Thread::sleep(time_t t){ return time_sleep(t);}


Semaphore::Semaphore(unsigned init): myHandle(nullptr){ sem_open(&myHandle, init);}
Semaphore::~Semaphore() {if(myHandle) {sem_close(myHandle); myHandle = nullptr;}}
int Semaphore::wait() { return myHandle ? sem_wait(myHandle) : -1;}
int Semaphore::signal() { return myHandle ? sem_signal(myHandle) : -1;}


char Console::getc() { return ::getc();}
void Console::putc(char c) { ::putc(c);}

PeriodicThread::PeriodicThread(time_t period): Thread(), period(period) {}

void PeriodicThread::terminate() { period = 0; }

void PeriodicThread::run() {
    while (period) {
        periodicActivation();
        if (period) Thread::sleep(period);
    }
}


 void Thread::addChild(Thread* child){
    // this->parent = 
    ::thread_add_child(&child->myHandle);
 }

 void Thread::joinAll(){
    ::thread_join_all();
 }