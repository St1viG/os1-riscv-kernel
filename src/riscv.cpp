#include "../h/riscv.hpp"

// naked, and out of the header, so ra still holds the return address into the
// caller when csrw runs.
__attribute__((naked)) void Riscv::popSppSpie(){
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}