#include "../h/syscall_cpp.hpp"
#include "Modification_test.hpp"
#include "printing.hpp"


bool finishedA = false;

class testThreadA: public Thread{
    void testThreadBodyA(void* arg);
public:
    testThreadA(): Thread() {}

    void run() override {
        testThreadBodyA(nullptr);
    }

};

class testThreadB: public Thread{
    void testThreadBodyB(void* arg);
public:
    testThreadB(): Thread() {}

    void run() override {
        testThreadBodyB(nullptr);
    }

};


class testThreadC: public Thread{
    void testThreadBodyC(void* arg);
public:
    testThreadC(): Thread() {}

    void run() override {
        testThreadBodyC(nullptr);
    }

};

void testThreadA::testThreadBodyA(void* arg){
    Thread* t[3];
    for(int i = 0; i < 3; i++)
        t[3] = new testThreadB();

    for(int i = 0; i < 3; i++)
        t[3]->start();

    for(int i = 0; i < 3; i++)
        addChild(t[3]);

    joinAll();

    printString("Thread A je gotov \n");
    finishedA = true;
}


void testThreadB::testThreadBodyB(void* arg){
    Thread* t[3];
    for(int i = 0; i < 3; i++)
        t[3] = new testThreadC();

    for(int i = 0; i < 3; i++)
        t[3]->start();

    for(int i = 0; i < 3; i++)
        addChild(t[3]);

    joinAll();

    printString("Thread B je gotov \n");
}


void testThreadC::testThreadBodyC(void* arg){

    printString("Thread C je gotov \n");
}








void JoinAll_test(){
    Thread* threadA = new testThreadA();
    printString("Thread A se pokrece\n");
    threadA->start();

    while(!finishedA)
        Thread::dispatch();
    
    printString("Test je gotov\n");
}
