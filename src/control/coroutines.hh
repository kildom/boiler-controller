#ifndef _COROUTINES_HH_
#define _COROUTINES_HH_

#include "global.hh"

// Korutyny dla ubogich:

#define WAIT_FOR(time, ...) _WAIT_FOR2(time, __LINE__, __COUNTER__, ##__VA_ARGS__, {});
#define _WAIT_FOR2(time, l, c, block, ...) _WAIT_FOR3(time, l, c, block)
#define _WAIT_FOR3(time, l, c, block) \
    timer.set(time); \
    resumeLabel = &&_resume_##l##_##c; \
    _resume_##l##_##c: \
    if (!timer.ready()) { block; return; } \

#define WAIT_UNTIL(cond, ...) _WAIT_UNTIL2(cond, __LINE__, __COUNTER__, ##__VA_ARGS__, {});
#define _WAIT_UNTIL2(cond, l, c, block, ...) _WAIT_UNTIL3(cond, l, c, block)
#define _WAIT_UNTIL3(cond, l, c, block) \
    resumeLabel = &&_resume_##l##_##c; \
    _resume_##l##_##c: \
    if (!(cond)) { block; return; } \

#endif
