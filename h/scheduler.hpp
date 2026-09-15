#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include "tcb.hpp"

class Scheduler{
public:
    static void put(_thread* t);

    static _thread* get();

    static void setIdle(_thread* t) { idle = t;}

private:
    static ThreadQueue ready;
    static _thread* idle;

    Scheduler() = delete;
};


#endif // __SCHEDULER_HPP__