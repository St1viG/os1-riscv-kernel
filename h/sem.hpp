#ifndef __SEM_HPP__
#define __SEM_HPP__

#include "../lib/hw.h"
#include "syscall_c.hpp"
#include "tcb.hpp"

class _sem {
public:
    // Negative on failure, as the spec requires; the exact codes are ours.
    enum Error {
        OK = 0,
        ERR_HANDLE = -1,        // null handle, already closed, or heap exhausted
        ERR_CLOSED = -2,        // closed out from under us while we were blocked
        ERR_OVERFLOW = -3,      // signal would wrap the counter
    };

    static int open(sem_t* handle, unsigned init);
    static int close(sem_t handle);
    static int wait(sem_t handle, unsigned n);
    static int signal(sem_t handle, unsigned n);


    // Not the global operator new: that one ecalls, and semaphores are
    // created inside the trap handler. Bytes here, blocks in the allocator.
    static void* operator new(size_t bytes);
    static void* operator new(size_t, void* place) noexcept { return place; }
    static void operator delete(void* ptr) noexcept;

private:
    explicit _sem(unsigned init);

    bool valid() const { return magic == MAGIC; }
    static const uint64 MAGIC = 0x5E3A0F0051A0BEEFUL;

    void releaseReady();

    void releaseAll();

    uint64 magic;
    unsigned value;         // units available; never negative
    ThreadQueue blocked;    // FIFO of waiters, each carrying its n in the TCB
};


#endif // __SEM_HPP__
