
#include "time.hh"
#include "deque.hh"
#include "lowlevel.h"

static const int MIN_UPDATE_INTERVAL = 500;

uint32_t Time::prev_lo = 0;
uint32_t Time::prev_hi = 0;
int Time::delta = 0;
uint64_t Time::time = 0;
uint64_t Time::scheduled_update = MIN_UPDATE_INTERVAL;
static Deque timers = { .first = nullptr };

uint64_t Time::real_time()
{
    auto now_lo = get_time();
    if ((now_lo ^ prev_lo) >> 31) {
        prev_hi++;
    }
    prev_lo = now_lo;
    return (uint64_t)prev_lo | (uint64_t)prev_hi << 32;
}

void Time::schedule(int time_relative)
{
    schedule_absolute(time + time_relative);
}

void Time::schedule_absolute(uint64_t time_absolute)
{
    if (time_absolute <= time) {
        time_absolute = time + 1;
    }
    if (time_absolute < scheduled_update) {
        scheduled_update = time_absolute;
        timeout((uint32_t)scheduled_update);
    }
}

void Time::update_start()
{
    auto now = real_time();
    delta = now - time;
    time = now;
    scheduled_update = now + MIN_UPDATE_INTERVAL;
    timeout((uint32_t)scheduled_update);

    auto timer = timers.get_first<Timer>();
    while (timer != nullptr) {
        auto next = Deque::get_next(timer);
        if (timer->ready()) {
            timers.remove(timer);
        } else {
            schedule_absolute(timer->time_absolute);
        }
        timer = next;
    }
}

Timer::Timer(): time_absolute(Time::NEVER)
{
}

void Timer::set(int time_relative)
{
    time_absolute = Time::time + time_relative;
    Time::schedule(time_relative);
    timers.add(this);
}

void Timer::clear()
{
    time_absolute = Time::NEVER;
    timers.remove(this);
}

bool Timer::ready()
{
    return time_absolute <= Time::time;
}

int Timer::overtime()
{
    return (int)Time::time - (int)time_absolute;
}
