#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/sem.hpp"

#include "../h/console.hpp"

// Kernel-side printing. Goes straight at the device, never through putc: this
// runs inside the trap handler, where an ecall would re-enter the trap we are
// reporting, and where the drainer thread cannot run to empty a buffer for us.
static void kprintString(char const* s) {
    while (*s) _console::putcSync(*s++);
}

static void kprintHex(uint64 x) {
    _console::putcSync('0');
    _console::putcSync('x');
    for (int shift = 60; shift >= 0; shift -= 4) {       // 16 nibbles, high to low
        uint64 nib = (x >> shift) & 0xF;
        _console::putcSync((char)(nib < 10 ? '0' + nib : 'a' + nib - 10));
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

            case 0x11: {
                thread_t* handle = (thread_t*)frame[REG_A1];
                _thread::Body body = (_thread::Body)frame[REG_A2];
                void* arg = (void*)frame[REG_A3];
                void* stackSpace = (void*)frame[REG_A4];

                void* stackBase = (char*)stackSpace - DEFAULT_STACK_SIZE;

                _thread* t = _thread::createThread(body, arg, stackBase, stackSpace, true);
                if(!t){
                    frame[REG_A0] = (uint64) - 1;
                    break;
                }
                Scheduler::put(t);
                if(handle)
                    *handle = t;
                frame[REG_A0] = 0;
                break;
            }
            case 0x12:
                _thread::exit();
                break;
            case 0x13:
                _thread::dispatch();
                break;
            case 0x21:
                frame[REG_A0] = (uint64)_sem::open((sem_t*)frame[REG_A1],(unsigned)frame[REG_A2]);
                break;
            case 0x22:
                frame[REG_A0] = (uint64)_sem::close((sem_t)frame[REG_A1]);
                break;
            case 0x23:
                frame[REG_A0] = (uint64)_sem::wait((sem_t)frame[REG_A1],1);
                break;
            case 0x24:
                frame[REG_A0] = (uint64)_sem::signal((sem_t)frame[REG_A1],1);
                break;
            case 0x25:
                frame[REG_A0] = (uint64)_sem::wait((sem_t)frame[REG_A1],(unsigned)frame[REG_A2]);
                break;
            case 0x26:
                frame[REG_A0] = (uint64)_sem::signal((sem_t)frame[REG_A1], (unsigned)frame[REG_A2]);
                break;
            case 0x31:
                frame[REG_A0] = (uint64)_thread::sleep((time_t)frame[REG_A1]);
                break;
            case 0x41:
                frame[REG_A0] = (uint64)_console::getc();
                break;
            case 0x42:
                _console::putc((char)frame[REG_A1]);
                break;
            // Kernel-internal, not one of the codes the spec prescribes: main()
            // uses it to drain the output buffer before it stops the machine.
            case 0x43:
                _console::flush();
                break;
            default:
                frame[REG_A0] = (uint64) - 1;
                break;
        }
    }else if(scause == Riscv::INT_SOFTWARE){
        // This board delivers the 10 Hz timer as a SOFTWARE interrupt, not as
        // stimer -- INT_TIMER is never seen. Acknowledge before dispatching:
        // sip.SSIP is a level bit, and tick() may not return to this thread
        // for a long time.
        Riscv::mc_sip(Riscv::SI_SSI);
        _thread::tick();
    }else if(scause == Riscv::INT_EXTERNAL){
        _console::handleIrq();
    }else{
        kprintString("unexpected trap, scause = ");
        kprintHex(scause);
        kprintString(" sepc = ");
        kprintHex(frame[FRAME_SEPC]);
        kprintString("\n");
        for (;;);
    }
}
