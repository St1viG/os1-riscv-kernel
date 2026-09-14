#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../lib/console.h"


// Kernel-side printing. Goes straight to console.lib, never through the C API:
// this runs inside the trap handler, where an ecall would re-enter the trap we
// are trying to report.
static void kprintString(char const* s) {
    while (*s) __putc(*s++);
}

static void kprintHex(uint64 x) {
    __putc('0');
    __putc('x');
    for (int shift = 60; shift >= 0; shift -= 4) {       // 16 nibbles, high to low
        uint64 nib = (x >> shift) & 0xF;
        __putc((char)(nib < 10 ? '0' + nib : 'a' + nib - 10));
    }
}





extern "C" void handleTrap(uint64 *frame){
    uint64 scause = Riscv::r_scause();

    if(scause == Riscv::ECALL_FROM_U || scause == Riscv::ECALL_FROM_S){
        frame[FRAME_SEPC] += 4;
        uint64 code = frame[REG_A0];
    
        switch(code){
            case 0x01:{
                void* p = MemoryAllocator::getInstance().mem_alloc(frame[REG_A1]);
                frame[REG_A0] = (uint64)p;
                break;
            }
            case 0x02: {
                int r = MemoryAllocator::getInstance().mem_free((void*)frame[REG_A1]);
                frame[REG_A0] = (uint64)r;
                break;
            }
            default:
                frame[REG_A0] = (uint64) - 1;
                break;
        }
    }else if(scause == Riscv::INT_SOFTWARE){
        Riscv::mc_sip(Riscv::SI_SSI);
    }else if(scause == Riscv::INT_EXTERNAL){
        console_handler();
    }else{
        kprintString("unexpected trap, scause = ");
        kprintHex(scause);
        kprintString(" sepc = ");
        kprintHex(frame[FRAME_SEPC]);
        kprintString("\n");
        for (;;);
    }
}
