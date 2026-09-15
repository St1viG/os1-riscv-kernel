#include "../h/riscv.hpp"

// sret pops SPP into the privilege level and SPIE into SIE as a side effect of
// jumping to sepc, so pointing sepc at our own return address drops the caller
// to user mode and re-enables interrupts in one instruction.
//
// Two things are load-bearing. The definition stays out of the header so this
// can never be inlined -- the trick needs ra to still hold the return address
// into the caller when csrw executes. And naked is not decoration: without it
// -Og -fno-omit-frame-pointer emits
//     addi sp,sp,-16 ; sd s0,8(sp) ; addi s0,sp,16
// ahead of the asm, and sret leaves before the matching epilogue, so the caller
// resumes with sp 16 bytes low and its frame pointer destroyed. Any local it
// holds across this call then reads garbage.
__attribute__((naked)) void Riscv::popSppSpie(){
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}