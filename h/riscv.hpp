#ifndef _RISCV_HPP_
#define _RISCV_HPP_

#include "../lib/hw.h"

extern "C" void trapVector();
extern "C" void handleTrap(uint64* frame);

enum FrameSlot {
    REG_RA = 1,  REG_SP = 2,
    REG_A0 = 10, REG_A1 = 11, REG_A2 = 12,    // syscall code and arguments
    REG_A3 = 13, REG_A4 = 14,
    FRAME_SEPC    = 32,
    FRAME_SSTATUS = 33,
    FRAME_SIZE    = 34,                       // * 8 == the 272 in trap.S
};

class Riscv{
public:
    enum BitMaskSstatus {
        SSTATUS_SIE = (1 << 1),
        SSTATUS_SPIE = (1 << 5),
        SSTATUS_SPP = (1 << 8),
    };

    static void popSppSpie();

    static uint64 r_sstatus() { uint64 v; __asm__ volatile("csrr %0, sstatus" : "=r"(v)); return v;}
    static void w_sstatus(uint64 v) { __asm__ volatile("csrw sstatus, %0" : : "r"(v));}
    static void ms_sstatus(uint64 m) { __asm__ volatile("csrs sstatus, %0" : : "r"(m));}
    static void mc_sstatus(uint64 m) { __asm__ volatile("csrc sstatus, %0" : : "r"(m));}

    static uint64 r_sepc() { uint64 v; __asm__ volatile("csrr %0, sepc" : "=r"(v)); return v;}
    static void w_sepc(uint64 v) { __asm__ volatile("csrw sepc, %0" : : "r"(v));}

    enum ScauseCode: uint64 {
        ECALL_FROM_U = 0x08UL,
        ECALL_FROM_S = 0x09UL,
        INT_FLAG     = (1UL << 63),
        INT_SOFTWARE = INT_FLAG | 1UL,
        INT_TIMER    = INT_FLAG | 5UL,   
        INT_EXTERNAL = INT_FLAG | 9UL,   
    };

    static uint64 r_scause() { uint64 v; __asm__ volatile("csrr %0, scause" : "=r"(v)); return v;}
    enum StvecMode { 
        DIRECT = 0x00, 
        VECTORED = 0x01 
    };
    static uint64 r_stvec() { uint64 v; __asm__ volatile("csrr %0, stvec" : "=r"(v)); return v;}
    static void w_stvec(uint64 v) { __asm__ volatile("csrw stvec, %0" : : "r"(v));}

    static uint64 r_stval() {uint64 v; __asm__ volatile("csrr %0, stval" : "=r"(v)); return v;}

    enum BitMaskSi {
        SI_SSI = (1 << 1),
        SI_STI = (1 << 5),
        SI_SEI = (1 << 9),
    };

    static uint64 r_sip() { uint64 v; __asm__ volatile("csrr %0, sip" : "=r"(v)); return v;}
    static void ms_sip(uint64 m) { __asm__ volatile("csrs sip, %0" : : "r"(m));}
    static void mc_sip(uint64 m) { __asm__ volatile("csrc sip, %0" : : "r"(m));}

    static uint64 r_sie() { uint64 v; __asm__ volatile("csrr %0, sie" : "=r"(v)); return v;}
    static void ms_sie(uint64 m) { __asm__ volatile("csrs sie, %0" : : "r"(m));}
    static void mc_sie(uint64 m) { __asm__ volatile("csrc sie, %0" : : "r"(m));}
};


#endif  //_RISCV_HPP_