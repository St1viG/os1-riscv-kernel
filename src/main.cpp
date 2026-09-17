#include "../lib/hw.h"
#include "../h/riscv.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/syscall_c.hpp"
#include "../h/console.hpp"

extern void userMain();

// ABI code 0x43, kernel-internal: drain the console. popSppSpie below drops
// main() to user mode for good, so it has to ask through the ABI like any other
// user code -- and without it the last line every test prints dies in the
// output buffer, because the store that halts the machine is immediate.
static void consoleFlush(){
    register uint64 a0 __asm__("a0") = 0x43;
    __asm__ volatile("ecall" : "+r"(a0) : : "memory");
}

static void idleBody(void*){
    for(;;)
        thread_dispatch();
}


int main(){

    Riscv::w_stvec((uint64)&trapVector | Riscv::DIRECT);

    _thread::running = _thread::createThread(nullptr,nullptr,nullptr,nullptr,true);
    if(!_thread::running)
        return -1;
    void* idleStack = mem_alloc(DEFAULT_STACK_SIZE);
    if(!idleStack)
        return -1;
    _thread* idle = _thread::createThread(idleBody, nullptr, idleStack, (char*)idleStack + DEFAULT_STACK_SIZE, false);
    if(!idle)
        return -1;
    Scheduler::setIdle(idle);

    _console::init();

    // State the two interrupt sources the kernel handles. The environment
    // already enables them -- console interrupts arrive today -- but csrs is
    // purely additive, and naming the dependency is cheaper than rediscovering
    // it. SI_STI is absent on purpose: this board delivers the timer as
    // SI_SSI, a software interrupt (see trap.cpp). Safe here specifically
    // because sstatus.SIE is still 0, so nothing is delivered until the sret
    // two lines down -- by which time running, idle and the scheduler exist.
    Riscv::ms_sie(Riscv::SI_SSI | Riscv::SI_SEI);

    Riscv::mc_sstatus(Riscv::SSTATUS_SPP);
    Riscv::ms_sstatus(Riscv::SSTATUS_SPIE);
    Riscv::popSppSpie();

    userMain();

    consoleFlush();

    *(volatile uint32*)0x100000 = 0x5555; //halts the emulator;
    return 0;
}