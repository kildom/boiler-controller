
#include <utility>

#include "menu.hh"
#include "temp.hh"
#include "log.hh"
#include "storage.hh"
#include "emergency.hh"

int const MIN_LEVEL = -0x80000000;

EmergencyDisable emergencyDisable;

static bool valid(int temp, int max, int min = -5) {
    if (temp == Temp::INVALID || temp < min || temp > max) {
        return false;
    } else {
        return true;
    }
}

static char* formatTemp(int temp) {
    static char buf[20];
    tempToStr(buf, temp);
    return buf;
}


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
/* ▁▁▁▁▇▁▁▁▁▁▁▇▇▇▇▇▇▇▇▁▁▁  input
 *    ▕ -T- ▏▕ -T- ▏
 * ▁▁▁▁▇▇▇▇▇▁▁▇▇▇▇▇▇▇▇▁▁▁  output
 */
};

class Fault: private DequeItem {
public:
    enum State {
        INACTIVE = 0,
        PRESENT = 1,
        ACTIVE = 2,
    };
    State state;
    int temp;
    const char* message;
    const int id;

    static Deque deque;

private:
    static int lastId;

public:
    Fault(const char* message):
        state(INACTIVE),
        temp(Temp::INVALID),
        message(message),
        id(lastId++) {}

    Fault& update(bool value) {
        if (value) {
            if (state == INACTIVE) {
                ERR("Fault: %s", message);
                deque.add(this);
            }
            state = ACTIVE;
        } else if (state == ACTIVE) {
            state = PRESENT;
        }
        return *this;
    }

    Fault& setMax(int newTemp) {
        if (state == ACTIVE) {
            if (newTemp > temp) {
                ERR("Max Temp: %d - %s", newTemp, message);
                temp = newTemp;
            }
        }
        return *this;
    }

    void clear() {
        if (state != INACTIVE) {
            DBG("End fault: %s", message);
            deque.remove(this);
            temp = Temp::INVALID;
            state = INACTIVE;
        }
    }

    bool get() {
        return state != INACTIVE;
    }
};

int Fault::lastId = 0;
Deque Fault::deque;

void emergencyUpdate() {

    static Fault elekTooHigh("Piec elekt. - nieprawidłowa temp.");
    elekTooHigh
        .update(!valid(Temp::piecElek(), storage->elekCritical))
        .setMax(Temp::piecElek());

    static Fault podl1Awaria("Podl. 1 - awaria");
    static DelayOnCondition delayPodl1;
    podl1Awaria
        .update(
            (storage->pelletDom && delayPodl1.get(Zawor::podl1.isFullyOpen() && Temp::piecPelet() > storage->podlFaultPiecTemp, storage->podlFaultDelay))
            || !valid(Temp::podl1(), storage->zaw_podl1.critical)
        )
        .setMax(Temp::podl1());

    static Fault podl2Awaria("Podl. 2 - awaria");
    static DelayOnCondition delayPodl2;
    podl2Awaria
        .update(
            (storage->pelletDom && delayPodl2.get(Zawor::podl2.isFullyOpen() && Temp::piecPelet() > storage->podlFaultPiecTemp, storage->podlFaultDelay))
            || !valid(Temp::podl2(), storage->zaw_podl2.critical)
        )
        .setMax(Temp::podl2());

    emergencyDisable.elek = elekTooHigh.get();
    emergencyDisable.podl1 = podl1Awaria.get();
    emergencyDisable.podl2 = podl2Awaria.get();
}

