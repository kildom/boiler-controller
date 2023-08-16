#ifndef _UTILS_HH_
#define _UTILS_HH_


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
