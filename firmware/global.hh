#ifndef _GLOBAL_HH_
#define _GLOBAL_HH_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lowlevel.hh"

constexpr int64_t operator ""_ms(unsigned long long x) { return x; };
constexpr int64_t operator ""_sec(unsigned long long x) { return x * 1000uLL; };
constexpr int64_t operator ""_min(unsigned long long x) { return x * 60uLL * 1000uLL; };
constexpr int64_t operator ""_h(unsigned long long x) { return x * 60uLL * 60uLL * 1000uLL; };

constexpr int operator ""_deg(unsigned long long x) { return (int)x * 100; };

#if defined(__GNUC__) && __GNUC__ >= 7
#define no_break __attribute__((fallthrough))
#else
#define no_break ((void)0)
#endif

static struct GlobalInit {
    GlobalInit() {
        global_init();
    }
    void init();
} _global_init_call;

#endif

