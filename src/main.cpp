#include "../lib/hw.h"

extern void userMain();

int main(){

    userMain();
    *(volatile uint32*)0x100000 = 0x5555; //halts the emulator;
    return 0;
}