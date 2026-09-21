
#include "../h/syscall_cpp.hpp"
#include "Modification_test.hpp"

#include "printing.hpp"

static Thread* threadA;
static Thread* threadB;
static Thread* threadC;

static char msgAtoB[] = "Nit A -> Nit B\n";
static char msgAtoC[] = "Nit A -> Nit C\n";
static char msgBtoC1[] = "Nit B -> Nit C #1\n";
static char msgBtoC2[] = "Nit B -> Nit C #2\n";
static char msgCtoB[] = "Nit C -> Nit B\n";
static char msgCtoA[] = "Nit C -> Nit A\n";
static volatile bool finishedA = false;
static volatile bool finishedB = false;
static volatile bool finishedC = false;



class MsgWorkerA: public Thread{
    void WorkerBodyA(void* arg);
public:
    MsgWorkerA(): Thread(){}

    void run() override{
        WorkerBodyA(nullptr);
    }
};

class MsgWorkerB: public Thread{
    void WorkerBodyB(void* arg);
public:
    MsgWorkerB():Thread() {}

    void run() override {
        WorkerBodyB(nullptr);
    }

};

class MsgWorkerC: public Thread{
    void WorkerBodyC(void* arg);
public:
    MsgWorkerC():Thread() {}

    void run() override {
        WorkerBodyC(nullptr);
    }

};

void MsgWorkerA::WorkerBodyA(void* arg){
    for(int i = 0; i < 5; i++){
        threadB->send(msgAtoB);
        threadC->send(msgAtoC);

        char* m = Thread::receive();
        if(m)
            printString(m);
    }
    printString("A finished\n");
    finishedA = true;
}

void MsgWorkerB::WorkerBodyB(void* arg){
    for(int i = 0; i < 5; i++){
        threadC->send(msgBtoC1);
        threadC->send(msgBtoC2);

        char* m = Thread::receive();
        if(m)
            printString(m);

        m = Thread::receive();
        if(m)
            printString(m);
    }
    printString("B finished\n");
    finishedB = true;
}

void MsgWorkerC::WorkerBodyC(void* arg){
    for(int i = 0; i < 5; i++){
        char* m = Thread::receive();
        if(m)
            printString(m);

        m = Thread::receive();
        if(m)
            printString(m);
        m = Thread::receive();
        if(m)
            printString(m);
        
        threadB->send(msgCtoB);
        threadA->send(msgCtoA);
    }
    printString("C finished\n");
    finishedC = true;
}



void Messages_test(){

    threadA = new MsgWorkerA();
    threadB = new MsgWorkerB();
    threadC = new MsgWorkerC();

    threadA->start();
    threadB->start();
    threadC->start();

    while(!(finishedA && finishedB && finishedC))
        Thread::dispatch();

    delete threadA;
    delete threadB;
    delete threadC;
    return;
}