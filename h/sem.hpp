#ifndef __SEM_HPP__
#define __SEM_HPP__

#include "../lib/hw.h"
#include "syscall_c.hpp"
#include "tcb.hpp"

// Counting semaphore -- the kernel-side object a sem_t handle points at.
//
// Same naming trick as _thread: syscall_c.hpp declares `class _sem;` and
// `typedef _sem* sem_t`, so naming the kernel class _sem keeps the trap handler
// free of casts -- what arrives in a1 for 0x22-0x26 is already a sem_t.
//
// Representation: `value` is the number of units AVAILABLE and never goes
// negative, with the waiters held in an explicit queue. The classic signed
// counter (where the negative magnitude *is* the waiter count) cannot express
// sem_wait_n: each waiter owes a different n, so one number cannot encode the
// backlog, and signal_n has to read those n's to decide how many threads it can
// release. This is the one place the 2026 spec rules out the textbook design.
class _sem {
public:
    // Negative on failure, as the spec requires; the exact codes are ours.
    enum Error {
        OK = 0,
        ERR_HANDLE = -1,        // null handle, already closed, or heap exhausted
        ERR_CLOSED = -2,        // closed out from under us while we were blocked
        ERR_OVERFLOW = -3,      // signal would wrap the counter
    };

    // The whole ABI surface. Static and handle-taking, so handle validation
    // lives in exactly one place and trap.cpp stays a pure dispatcher.
    // sem_wait/sem_signal are simply n == 1 -- not a special case, just the
    // common one, which is why there is no second implementation of either.
    static int open(sem_t* handle, unsigned init);
    static int close(sem_t handle);
    static int wait(sem_t handle, unsigned n);
    static int signal(sem_t handle, unsigned n);


    // Same reasoning as _thread (see tcb.hpp): a semaphore is created from
    // inside the trap handler, and the global operator new is an ecall -- a
    // nested trap there would clobber sepc. Straight to the allocator instead,
    // and note the unit change: MemoryAllocator counts BLOCKS, operator new is
    // handed BYTES. The throwing form cannot be trusted to report failure, so
    // open() allocates, tests, and only then constructs via the placement form.
    static void* operator new(size_t bytes);
    static void* operator new(size_t, void* place) noexcept { return place; }
    static void operator delete(void* ptr) noexcept;

private:
    explicit _sem(unsigned init);

    // Best-effort stale-handle guard, in the same style as MemoryAllocator's
    // header magic: close() clears it before freeing, so a wait/signal on an
    // already-closed handle reports an error instead of blocking a thread on
    // freed memory. Best-effort only -- once the block is handed out again the
    // bytes belong to someone else. The case the spec actually mandates (closed
    // WHILE waiting) is handled exactly, through the TCB, not through this.
    bool valid() const { return magic == MAGIC; }
    static const uint64 MAGIC = 0x5E3A0F0051A0BEEFUL;

    // Head-blocking release: stop at the first waiter the counter cannot
    // satisfy in full rather than skipping to a cheaper request behind it.
    // Skipping would raise throughput but lets a large n starve indefinitely
    // behind a stream of small ones.
    void releaseReady();

    // Wake every waiter with ERR_CLOSED. Their result goes into their own TCBs
    // because close() frees this object before any of them runs again.
    void releaseAll();

    uint64 magic;
    unsigned value;         // units available; never negative
    ThreadQueue blocked;    // FIFO of waiters, each carrying its n in the TCB
};


#endif // __SEM_HPP__
