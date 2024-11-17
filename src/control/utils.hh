#ifndef _UTILS_HH_
#define _UTILS_HH_

#include "global.hh"
#include "time.hh"

template<class P, class M>
constexpr size_t _offsetOfImpl(const M P::*member)
{
    return (size_t)&(reinterpret_cast<P*>(0)->*member);
    //return (size_t)&((P*)(void*)(0)->*member);
}

template<class P, class M>
constexpr P* _containerOfImpl(M* ptr, const M P::*member)
{
    return (P*)( (char*)ptr - _offsetOfImpl(member));
}

template<class P, class M>
constexpr P* _containerOfImpl(const M* ptr, const M P::*member)
{
    return (P*)( (char*)ptr - _offsetOfImpl(member));
}

#define offsetOf(member) _offsetOfImpl(&member)
#define containerOf(ptr, member) _containerOfImpl(ptr, &member)


class OneTimeFlag {
private:
    bool on;
public:
    OneTimeFlag(bool on = false): on(on) {}

    bool getAndReset() {
        bool res = on;
        on = false;
        return res;
    }

    bool get() const {
        return on;
    }

    void set() {
        on = true;
    }

    void reset() {
        on = false;
    }

    operator bool() const {
        return on;
    }
};


class DelayOnCondition {
/* ▁▁▁▁▇▁▁▁▇▇▇▇▇▇▇▇▁▁▁▁▁▁  input
 *        ▕ -T- ▏
 * ▁▁▁▁▁▁▁▁▁▁▁▁▁▇▇▇▁▁▁▁▁▁  output
 */
private:
    uint64_t onTime;

public:
    DelayOnCondition(): onTime(Time::time) {}

    bool get(bool input, int time) {
        if (input) {
            if (Time::time >= onTime + time) {
                return true;
            }
        } else {
            onTime = Time::time;
        }
        return false;
    }
};

class DelayOffCondition {
/* ▁▁▁▁▇▁▇▁▁▁▁▇▁▁▇▇▇▇▇▁▁▁  input
 *    ▕ -T- ▏▕ -T- ▏
 * ▁▁▁▁▇▇▇▇▇▁▁▇▇▇▇▇▇▇▇▁▁▁  output
 */
private:
    uint64_t onTime;
    bool prev;

public:
    DelayOffCondition(): onTime(0), prev(false) {}

    bool get(bool input, int time) {
        bool active = Time::time < onTime;
        if (!prev && input && !active) {
            onTime = Time::time + time;
        }
        prev = input;
        return input || active;
    }

    void triggerNow(int time) {
        onTime = Time::time + time;
    }
};

#endif // _UTILS_HH_
