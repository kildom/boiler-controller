#ifndef __ModelCommon_HH
#define __ModelCommon_HH

#if defined(__ARM_ARCH) || defined(__arm__) || defined(USE_FLOAT32)
typedef float fptype;
#else
typedef double fptype;
#endif

constexpr fptype operator ""_f(long double x) { return fptype(x); }

#endif
