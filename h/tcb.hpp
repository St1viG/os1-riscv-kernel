#ifndef __TCB_HPP__
#define __TCB_HPP__

#include "../lib/hw.h"
#include "syscall_c.hpp"
#include "sem.hpp"

class _thread {
public:
    using Body = void (*)(void*);

    struct Context {
        uint64 ra;
        uint64 sp;
        uint64 s[3];
    };

    static _thread* createThread(Body body, void* arg,
                                 void* stackBase, void* stackSpace,
                                 bool userMode);

    static void dispatch();

    static void send(thread_t handle, char* message);

    static char* receive();

    static void exit();

    static void tick();

    static int sleep(time_t ticks);

    static _thread* running;

    bool isFinished() const { return finished; }
    bool isBlocked() const { return blocked; }
    void setBlocked(bool value) { blocked = value; }

    bool isUserMode() const { return userMode; }
    uint64 getTimeSlice() const { return timeSlice; }

    // Not the global operator new: that one ecalls, and TCBs are allocated
    // inside the trap handler. Bytes here, blocks in the allocator.
    static void* operator new(size_t bytes);
    static void* operator new(size_t, void* place) noexcept { return place; }
    static void operator delete(void* ptr) noexcept;

private:
    _thread(Body body, void* arg, void* stackBase, void* stackSpace, bool userMode);

    static void threadWrapper();

    static void reap();

    // Advances the sleep list by one tick and readies everything now due.
    static void wakeSleepers();

    Context context;
    Body body;
    void* arg;

    void* stackBase;        // exactly what mem_alloc returned; nullptr if bodiless
    bool userMode;          // false -> SPP stays 1, for kernel-internal threads
    bool finished;
    bool blocked;
    unsigned semRequest;    // units this thread is blocked waiting for
    int semResult;          // how its wait ended: 0, or a _sem::Error
    uint64 timeSlice;       // Phase 5: quantum remaining, in timer periods
    uint64 sleepTime;       // gap to the previous node in the sleep list
    _thread* next;

    // Parked here by exit() and released by the next thread to be switched in.
    static _thread* zombie;

    static _thread* sleepHead;

    char* message;
    _sem empty;
    _sem full;

    friend class ThreadQueue;
    friend class _sem;
};

using TCB = _thread;


extern "C" void contextSwitch(_thread::Context* oldContext,
                              _thread::Context* newContext);

#endif // __TCB_HPP__
