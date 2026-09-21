#include "../h/syscall_cpp.hpp"

#include "printing.hpp"

long _id = 0;
const int X = 9;
volatile int  finishedCnt = 0;

class BarWorker: public Thread{
    int id;
    void BarWorkerBody(void* arg);
public:
    BarWorker(): Thread(), id(++_id) {}

    void run() override {
        BarWorkerBody(nullptr);
    }
};

void BarWorker::BarWorkerBody(void* arg){
    for(int b = 0; b < 3; b++){
        //iteration
        for(int i = 0; i < 1000 * id; i++){
            for(int j = 0; j < 10000; j++);
            thread_dispatch();
        }

        barrier();

        printString("id:");
        printInt(id);
        printString(" iteracija:");
        printInt(b);
        printString("\n");
    }

    finishedCnt++;
}


void Barrier_test(){
    Thread* t[X];
    for(int i = 0; i < X; i++){
        t[i] = new BarWorker();
    }
    for(int i = 0; i < X; i++)
        t[i]->start();

    while(finishedCnt != X){
        Thread::dispatch();
    }
    
    for (auto thread: t) { delete thread; }
}