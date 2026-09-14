#include "../lib/hw.h"
#include "../h/riscv.hpp"
extern void userMain();

int main(){

    Riscv::w_stvec((uint64)&trapVector | Riscv::DIRECT);
    userMain();
    *(volatile uint32*)0x100000 = 0x5555; //halts the emulator;
    return 0;
}