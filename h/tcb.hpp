#ifndef __TCB_HPP__
#define __TCB_HPP__

#include "../lib/hw.h"
#include "syscall_c.hpp"

// Thread Control Block -- the kernel-side object a thread_t handle points at.
//
// The spec names the handle's target _thread (syscall_c.hpp: typedef _thread*
// thread_t), so the TCB *is* _thread. That keeps the trap handler free of casts:
// what arrives in a1 for 0x11 is already a _thread**. TCB below is an alias, so
// kernel code can use the conventional name without a second type existing.
class _thread {
public:
    using Body = void (*)(void*);

    struct Context {
        uint64 ra;
        uint64 sp;
    };

    // stackSpace is the TOP of the region the C API already allocated and passed
    // as ABI argument 4 -- the kernel never allocates a user stack itself.
    //
    // body == nullptr builds a bodiless TCB that owns no stack: used for the
    // context main() is already running on, and for the idle thread. Pass the
    // stack base separately so exit() can hand mem_free the exact pointer
    // mem_alloc returned; deriving it as stackSpace - DEFAULT_STACK_SIZE works
    // only while every stack is the same size.
    static _thread* createThread(Body body, void* arg,
                                 void* stackBase, void* stackSpace,
                                 bool userMode);

    // Picks the next ready thread and switches to it. The caller decides whether
    // `running` goes back in the ready queue by setting blocked/finished first.
    static void dispatch();

    // Marks the running thread finished and switches away for good. Does not
    // return. The TCB and its stack are released by whoever runs next -- a
    // thread cannot free the stack it is standing on.
    static void exit();

    static _thread* running;

    bool isFinished() const { return finished; }
    bool isBlocked() const { return blocked; }
    void setBlocked(bool value) { blocked = value; }

    bool isUserMode() const { return userMode; }
    uint64 getTimeSlice() const { return timeSlice; }

    // Kernel objects must not use the global operator new: that one calls the C
    // API mem_alloc, which is an ecall, and the kernel allocates TCBs from
    // inside the trap handler -- a nested ecall there would clobber sepc.
    // Straight to the allocator instead. Note the unit change: MemoryAllocator
    // counts BLOCKS, operator new is handed BYTES.
    // Careful: this is the *throwing* form, which the compiler is entitled to
    // assume never returns null -- it emits no check and runs the constructor
    // regardless. Verified: g++ -Og produces zero branches around the call. So
    // createThread allocates, tests, and only then constructs, via the
    // placement new below. Heap exhaustion is exactly what the hidden stress
    // tests provoke, so this is not a theoretical concern.
    static void* operator new(size_t bytes);
    static void* operator new(size_t, void* place) noexcept { return place; }
    static void operator delete(void* ptr) noexcept;

private:
    _thread(Body body, void* arg, void* stackBase, void* stackSpace, bool userMode);

    // Every thread starts here rather than at its body, because a body that
    // simply returns must still shut the thread down (spec: the wrapper may
    // never return). Also the one place user threads drop to user mode.
    static void threadWrapper();

    // Frees the thread that died before we were switched in. Called on the
    // resumed thread's stack, which is why it is safe.
    static void reap();

    Context context;
    Body body;
    void* arg;

    void* stackBase;        // exactly what mem_alloc returned; nullptr if bodiless
    bool userMode;          // false -> SPP stays 1, for kernel-internal threads
    bool finished;
    bool blocked;           // Phase 4: parked on a semaphore
    uint64 timeSlice;       // Phase 5: quantum in timer periods

    // Intrusive queue link. The spec asks for the chaining pointer to live in
    // the thread structure rather than in separately allocated list nodes, to
    // avoid the overhead and fragmentation of one allocation per queue push.
    // Safe with a single pointer only because the states are mutually exclusive:
    // a thread is ready XOR blocked XOR sleeping, never two at once.
    _thread* next;

    // Parked here by exit() and released by the next thread to be switched in.
    static _thread* zombie;

    friend class Scheduler;
};

using TCB = _thread;

// Placement new. Normally <new> supplies it, but a freestanding kernel has no
// standard library -- one line is cheaper than the alternative of trusting the
// throwing operator new not to hand back null.



// Defined in src/contextSwitch.S. extern "C" on purpose: the lecture version
// exports the hand-mangled _ZN3TCB13contextSwitchEPNS_7ContextES1_, which stops
// matching the moment the class or the nested struct is renamed -- and reports
// it as an undefined reference with no hint as to why.
extern "C" void contextSwitch(_thread::Context* oldContext,
                              _thread::Context* newContext);

#endif // __TCB_HPP__
