#include "../h/console.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/scheduler.hpp"
#include "../h/sem.hpp"
#include "../h/tcb.hpp"

static inline volatile char* txData() { return (volatile char*)CONSOLE_TX_DATA; }
static inline volatile char* rxData() { return (volatile char*)CONSOLE_RX_DATA; }
static inline volatile char* status() { return (volatile char*)CONSOLE_STATUS; }

static inline volatile char* ier() { return (volatile char*)(CONSOLE_TX_DATA + 1); }
static const char IER_RX_ONLY = 0x01;
static const char IER_NONE = 0x00;

static inline bool txReady() { return (*status() & CONSOLE_TX_STATUS_BIT) != 0; }
static inline bool rxReady() { return (*status() & CONSOLE_RX_STATUS_BIT) != 0; }


char _console::outBuf[CAPACITY];
size_t _console::outHead = 0;
size_t _console::outTail = 0;
sem_t _console::outSpace = nullptr;
sem_t _console::outItems = nullptr;

char _console::inBuf[CAPACITY];
size_t _console::inHead = 0;
size_t _console::inTail = 0;
sem_t _console::inItems = nullptr;

_thread* _console::drainer = nullptr;

void _console::init(){
    if(!bringUp())
        *ier() = IER_NONE;
}

bool _console::bringUp(){
    if(_sem::open(&outSpace, CAPACITY) != _sem::OK) return false;
    if(_sem::open(&outItems, 0) != _sem::OK) return false;
    if(_sem::open(&inItems, 0) != _sem::OK) return false;

    void* stack = MemoryAllocator::getInstance().mem_alloc(DEFAULT_STACK_SIZE / MEM_BLOCK_SIZE);
    if(!stack)
        return false;

    _thread* t = _thread::createThread(drainBody, nullptr, stack, (char*)stack + DEFAULT_STACK_SIZE, false);

    if(!t){
        MemoryAllocator::getInstance().mem_free(stack);
        return false;
    }

    Scheduler::put(t);

    // hw.lib's uartinit left IER at 0x03 -- receive enable AND
    // transmit-holding-register-empty enable -- for xv6's own interrupt-driven
    // transmitter. Ours is the polled thread above, so the transmit half is an
    // interrupt source nothing has a use for: receive only from here on.
    *ier() = IER_RX_ONLY;

    drainer = t;
    return true;
}

void _console::drainBody(void*){
    for(;;){
        // The C API on purpose, not _sem::wait: a kernel thread runs with
        // SIE = 1 and outside any trap, so touching the scheduler's queues
        // directly from here would race the timer interrupt. The ecall puts
        // that work back inside a trap, where interrupts are already masked.
        if(sem_wait(outItems) != 0){
            thread_dispatch();
            continue;
        }

        char c = outBuf[outHead];
        outHead = (outHead + 1) % CAPACITY;

        while(!txReady()) {}
        *txData() = c;

        sem_signal(outSpace);
    }
}

void _console::putc(char c){
    if(!drainer){
        putcSync(c);
        return;
    }

    if(_sem::wait(outSpace, 1) != _sem::OK){
        putcSync(c);
        return;
    }

    outBuf[outTail] = c;
    outTail = (outTail + 1) % CAPACITY;
    _sem::signal(outItems, 1);
}

int _console::getc(){
    if(!drainer)
        return getcSync();

    if(_sem::wait(inItems, 1) != _sem::OK)
        return EOF;

    char c = inBuf[inHead];
    inHead = (inHead + 1) % CAPACITY;

    putc(c);

    return (int)(unsigned char)c;
}

void _console::flush(){
    if(!drainer)
        return;

    if(_sem::wait(outSpace, CAPACITY) == _sem::OK)
        _sem::signal(outSpace, CAPACITY);
}

void _console::handleIrq(){
    int irq = plic_claim();

    if(irq == (int)CONSOLE_IRQ){
        while(rxReady()){
            char c = *rxData();
            if(c == '\r') c = '\n';

            size_t nextTail = (inTail + 1) % CAPACITY;
            if(nextTail == inHead)
                continue;
            inBuf[inTail] = c;
            inTail = nextTail;

            if(inItems)
                _sem::signal(inItems, 1);
        }
    }

    if(irq)
        plic_complete(irq);
}

void _console::putcSync(char c){
    while(!txReady()) {}
    *txData() = c;
}

int _console::getcSync(){
    while(!rxReady()) {}
    return (int)(unsigned char)*rxData();
}