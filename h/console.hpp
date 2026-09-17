#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include "../lib/hw.h"
#include "syscall_c.hpp"

class _console {
public:
    static void init();

    static void putc(char c);
    static int getc();
    static void flush();

    static void handleIrq();

    static void putcSync(char c);

private:
    static bool bringUp();

    static void drainBody(void*);

    static int getcSync();

    static const size_t CAPACITY = 256;

    static char outBuf[CAPACITY];
    static size_t outHead, outTail;
    static sem_t outSpace;
    static sem_t outItems;

    static char inBuf[CAPACITY];
    static size_t inHead, inTail;
    static sem_t inItems;

    static _thread* drainer;

    _console() = delete;
};

#endif // __CONSOLE_HPP__