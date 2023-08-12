#ifndef _TIMER_HH_
#define _TIMER_HH_

#include "global.hh"
#include "deque.hh"

class Time {
private:
    static uint32_t prev_lo;
    static uint32_t prev_hi;
    static uint64_t scheduled_update;
public:
    static const uint64_t NEVER = 0x3FFFFFFFFFFFFFFFuLL;
    static uint64_t real_time();
    static int delta;
    static uint64_t time;
    static void schedule(int time_relative);
    static void schedule_absolute(uint64_t time_absolute);
    static void update_start();
};

class Timer: private DequeItem {
private:
    uint64_t time_absolute;
public:
    Timer();
    void set(int time_relative);
    void clear();
    bool ready();
    int overtime();
    friend void Time::update_start();
};

#endif
