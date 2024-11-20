#ifndef PODL_HH_
#define PODL_HH_

#include "global.hh"

#include "zawor.hh"
#include "temp.hh"
#include "emergency.hh"

class Podl {
private:
    enum {
        ENABLED,
        DISABLED,
        ENABLING,
    } podlState;
    enum {
        OPEN,
        CLOSED,
        SIGNAL,
    } zaworState;
    int index;
    bool pompaOn;

public:

    Podl(int index): podlState(ENABLED), zaworState(SIGNAL), index(index), pompaOn(false) {}
    
    void update() {
        switch (podlState)
        {
        case ENABLING:
            if (Zawor::podl[index]->ready()) {
                Relay::pompa_podl(index, pompaOn);
                podlState = ENABLED;
            }
            no_break;
        case ENABLED:
            if (emergencyDisable.podl[index]) {
                Zawor::podl[index]->reset(-1, true);
                Relay::pompa_podl(index, false);
                podlState = DISABLED;
            }
            break;
        case DISABLED:
            if (!emergencyDisable.podl[index]) {
                Zawor::podl[index]->reset(zaworState == OPEN ? +1 : -1, true);
                podlState = ENABLING;
            }
        }
    }


    void reset(int dir, bool full) {
        if (podlState != DISABLED) {
            Zawor::podl[index]->reset(dir, full);
            Zawor::podl[index]->signal(0);
        }
        zaworState = dir > 0 ? OPEN : CLOSED;
    }

    void signal(int value) { // just set state if emergency disable
        Zawor::podl[index]->signal(value);
        zaworState = SIGNAL;
    }

    bool ready() { // zawsze true, gdy emergency disable
        if (podlState == ENABLED) {
            return Zawor::podl[index]->ready();
        } else {
            return podlState == DISABLED;
        }
    }

    void pompa(bool enable)
    {
        if (podlState == ENABLED) {
            Relay::pompa_podl(index, enable);
        }
        pompaOn = enable;
    }

    bool isFullyOpen() { // zależy od stanu, gdy emergency disable
        if (podlState == ENABLED) {
            return Zawor::podl[index]->isFullyOpen();
        } else {
            return zaworState == OPEN;
        }
    }

    int temp() { // zawsze równa zadanej, gdy emergency disable
        if (podlState != DISABLED) {
            return Temp::podl(index);
        } else {
            return Zawor::podl[index]->storage.temp;
        }
    }

    bool isDisabled() {
        return podlState == DISABLED;
    }

    static Podl podl1;
    static Podl podl2;
};

#endif /* PODL_HH_ */
