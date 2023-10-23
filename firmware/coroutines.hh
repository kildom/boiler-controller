#ifndef _COROUTINES_HH_
#define _COROUTINES_HH_

// Korutyny dla ubogich:

#define WAIT_FOR(time) _WAIT_FOR2(time, __LINE__, __COUNTER__);
#define _WAIT_FOR2(time, l, c) _WAIT_FOR3(time, l, c)
#define _WAIT_FOR3(time, l, c) \
    timer.set(time); \
    resumeLabel = &&_resume_##l##_##c; \
    _resume_##l##_##c: \
    if (!timer.ready()) return; \

#define WAIT_UNTIL(cond) _WAIT_UNTIL2(cond, __LINE__, __COUNTER__);
#define _WAIT_UNTIL2(cond, l, c) _WAIT_UNTIL3(cond, l, c)
#define _WAIT_UNTIL3(cond, l, c) \
    resumeLabel = &&_resume_##l##_##c; \
    _resume_##l##_##c: \
    if (!(cond)) return; \

#endif
