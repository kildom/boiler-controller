
#include "time.hh"
#include "deque.hh"
#include "lowlevel.h"

#define LOW_LEVEL_END ((uint32_t)(1 << TIME_BITS))
#define LOW_LEVEL_MASK (LOW_LEVEL_END - (uint32_t)1)

uint64_t Time::prev = 0;
int Time::delta = 0;
uint64_t Time::time = 0;
uint64_t Time::scheduled_update = PERIODIC_TIMEOUT;
static Deque timers = { .first = nullptr };

uint64_t Time::real_time()
{
    uint32_t now = get_time();
    int32_t diff = (int32_t)((now << (32 - TIME_BITS)) - ((uint32_t)prev << (32 - TIME_BITS))) >> (32 - TIME_BITS);
    prev += (int64_t)diff;
    return prev;
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
        timeout((uint32_t)scheduled_update & LOW_LEVEL_MASK);
    }
}

void Time::update_start()
{
    auto now = real_time();
    delta = now - time;
    time = now;
    scheduled_update = now + PERIODIC_TIMEOUT;

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

Timer::Timer(uint64_t initial_time): time_absolute(initial_time)
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
