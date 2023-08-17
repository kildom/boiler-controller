#ifndef _UTILS_HH_
#define _UTILS_HH_

#include <stdlib.h>

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


#endif // _UTILS_HH_
