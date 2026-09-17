// Ad-hoc harness for PeriodicThread (ROADMAP 8.8). No public test instantiates
// the class, so run(), terminate() and the period-doubles-as-terminate-flag
// trick from 7.4 are otherwise untested. Throwaway-tree code: it is not part of
// the submission and it does not have to be pretty.

#include "../h/syscall_cpp.hpp"
#include "printing.hpp"

// The only clock user code has is time_sleep itself, so one thread sleeps a
// tick at a time and counts. Accurate to about +/-1 tick under light load,
// which is all that is needed to tell a period of 3 from a period of 1. Both
// this thread and the threads under test sit on the same sleep list, so any
// drift applies to both and the *ratios* stay honest.
static volatile time_t clockTicks = 0;
static volatile bool clockRun = true;

static void clockBody(void*) {
    while (clockRun) {
        time_sleep(1);
        clockTicks++;
    }
}

static void report(char const* label, int value) {
    printString(label);
    printInt(value);
    printString("\n");
}

// ---------------------------------------------------------------- case A
// Fires at its period and stops itself from inside the activation, which is
// the path the `if (period)` guard in PeriodicThread::run exists for.
class SelfStopping : public PeriodicThread {
public:
    SelfStopping(time_t p, int limit) : PeriodicThread(p), fired(0), done(false), limit(limit) {}

    volatile int fired;
    volatile bool done;
    time_t stamp[8];

protected:
    void periodicActivation() override {
        if (fired < 8) stamp[fired] = clockTicks;
        fired++;
        if (fired >= limit) {
            terminate();
            done = true;
        }
    }

private:
    int limit;
};

// ---------------------------------------------------------------- cases B/C/D
// Plain counter, terminated from the outside (or never started properly,
// for the period == 0 case).
class Ticker : public PeriodicThread {
public:
    explicit Ticker(time_t p) : PeriodicThread(p), fired(0) {}
    volatile int fired;
protected:
    void periodicActivation() override { fired++; }
};

void periodicThreadHarness() {
    thread_t clock;
    thread_create(&clock, clockBody, nullptr);

    printString("PT: harness start\n");

    // -------- A: five activations at period 3, self-terminated
    SelfStopping a(3, 5);
    a.start();
    time_sleep(25);                     // 5 * 3 = 15 ticks of work, plus slack
    report("PT: A fired (want 5) = ", a.fired);
    printString("PT: A stamps =");
    for (int i = 0; i < a.fired && i < 8; i++) {
        printString(" ");
        printInt((int)a.stamp[i]);
    }
    printString("\n");
    printString("PT: A deltas (want ~3) =");
    for (int i = 1; i < a.fired && i < 8; i++) {
        printString(" ");
        printInt((int)(a.stamp[i] - a.stamp[i - 1]));
    }
    printString("\n");
    time_sleep(15);                     // long enough for a sixth activation
    report("PT: A fired after another 15 ticks (want 5) = ", a.fired);

    // -------- B: terminated from another thread while it sleeps
    Ticker b(2);
    b.start();
    time_sleep(9);
    int before = b.fired;
    b.terminate();
    time_sleep(15);
    report("PT: B fired before terminate (want ~4) = ", before);
    report("PT: B fired 15 ticks after terminate (want same) = ", b.fired);

    // -------- C: period 0 -- what the terminate-flag trick costs
    Ticker c(0);
    c.start();
    time_sleep(10);
    report("PT: C fired with period 0 (want 0) = ", c.fired);

    // -------- D: two periods at once, on the same sleep list
    Ticker d2(2), d5(5);
    d2.start();
    d5.start();
    time_sleep(30);
    d2.terminate();
    d5.terminate();
    report("PT: D period-2 fired (want ~15) = ", d2.fired);
    report("PT: D period-5 fired (want ~6) = ", d5.fired);
    time_sleep(10);

    clockRun = false;
    time_sleep(5);
    report("PT: clock ticks counted = ", (int)clockTicks);
    printString("PT: harness end\n");
}
