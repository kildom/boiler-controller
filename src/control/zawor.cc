#include  <algorithm>

#include "global.hh"
#include "coroutines.hh"
#include "relays.hh"
#include "time.hh"
#include "storage.hh"
#include "zawor.hh"
#include "log.hh"


static const int ZAW_RELAY_OFF_TIMEOUT = 300;
static const int ZAW_RESET_WORK_TIME_MUL = 10;

static const uint32_t ZAW_PRZERWA_PRZEK = 500_ms;
static const uint32_t RESET_OVERDRIVE = 4_sec;


Zawor::Zawor(Storage &storage, Relay::Index relayOn, Relay::Index relayPlus):
    storage(storage),
    relayOn(relayOn),
    relayPlus(relayPlus),
    signalValue(0),
    signalPos(256 * storage.czas_min_otwarcia),
    realPos(storage.czas_min_otwarcia),
    totalWorkTime(0),
    activeEvent(INITIAL),
    oldEvent(0),
    resumeLabel(nullptr)
{
}

void Zawor::handler(int event)
{
    int dirRequested;

    if (event) {
        if (oldEvent) {
            auto tmp = oldEvent;
            oldEvent = 0;
            sendEvent(tmp);
        }
        switch (event & 0xFF) {
            case INITIAL:
                resumeLabel = &&initial;
                break;
            case NORMAL:
                resumeLabel = &&normal;
                break;
            case RESET:
                resumeLabel = &&reset;
                break;
            case RESET_FULL:
                resumeLabel = &&resetFull;
                break;
            case FORCE_START:
                oldEvent = activeEvent;
                resumeLabel = &&force;
                break;
            case FORCE_STOP:
                // Already done before switch
                break;
        }
        activeEvent = event;
        Time::schedule(0);
        return;
    }


    dirRequested = (activeEvent & FLAG_PLUS) ? +1 : -1;

    int timeDelta = std::min(Time::delta, 255);
    if (timeDelta < 1) {
        Time::schedule(0);
        return;
    }

    if (signalValue != 0) {
        signalPos += signalValue * storage.czas_pracy_max * timeDelta // +-512 * 16_sec * 255 < 2^31
            / (storage.czas_przerwy + storage.czas_pracy_max);
        if (signalPos < 256 * storage.czas_min_otwarcia) {
            signalPos = 256 * storage.czas_min_otwarcia;
        } else if (signalPos > 256 * (storage.czas_otwarcia - storage.czas_min_otwarcia)) {
            signalPos = 256 * (storage.czas_otwarcia - storage.czas_min_otwarcia);
        }
    }

    if (realDirection != 0) {
        realPos += realDirection * timeDelta;
        totalWorkTime += std::abs(realDirection) * timeDelta;
    }

    if (expectedDirection != realDirection && switchTimer.ready()) {
        setRealDirection();
    }

    if (resumeLabel != NULL) {
        goto *resumeLabel;
    }

initial:
    activeEvent = INITIAL;
    setDirection(0);
    WAIT_UNTIL(false);

normal:
    activeEvent = NORMAL;
    setDirection(0);
    while (true) {
        WAIT_UNTIL((dirRequested = getRequestedDir()) != 0);  // Czekaj na zażądanie włączenia siłownika
        activeEvent = NORMAL | (dirRequested < 0 ? FLAG_MINUS : FLAG_PLUS);
        WAIT_UNTIL(setDirection(dirRequested));                 // Włącz siłownik
        WAIT_FOR(storage.czas_pracy_min);                  // Pozostaw włączony przynajmniej na min. czas pracy
        timer.set(storage.czas_pracy_max);
        WAIT_UNTIL(posUpdateStop(dirRequested) || timer.ready());    // Pozostaw wł. aż minie max czas pracy lub nie trzeba już więcej updatować
        if (longWorkSyncReq(dirRequested)) {
            sendEvent(RESET | (dirRequested < 0 ? FLAG_MINUS : FLAG_PLUS));
            return;
        }
        setDirection(0);                                   // Wyłącz siłownik
        // Wprowadź korektę sterowania (czas na rozruchu i/lub bezwładność siłownika)
        realPos = std::min(std::max(realPos + dirRequested * storage.korekta, 0), storage.czas_otwarcia);
    }

resetFull:
    WAIT_UNTIL(setDirection(dirRequested));
    WAIT_FOR(storage.czas_otwarcia + std::max((int)RESET_OVERDRIVE, storage.czas_otwarcia / 8));
    WAIT_UNTIL(setDirection(-dirRequested));// setDirection sprawdza ZAW_PRZERWA_PRZEK (przerwa nie jest wliczana do czasu pracy)
    WAIT_FOR(storage.czas_min_otwarcia);
    setDirection(0);
    if (dirRequested < 0) {
        realPos = storage.czas_min_otwarcia;
    } else {
        realPos = storage.czas_otwarcia - storage.czas_min_otwarcia;
    }
    signalPos = 256 * realPos;
    signalValue = 0;
    totalWorkTime = 0;
    goto normal;

reset:
    WAIT_UNTIL(setDirection(dirRequested));
    WAIT_FOR(RESET_OVERDRIVE + (dirRequested < 0 ? realPos : storage.czas_otwarcia - realPos));
    WAIT_UNTIL(setDirection(-dirRequested));
    WAIT_FOR(storage.czas_min_otwarcia);
    setDirection(0);
    if (dirRequested < 0) {
        realPos = storage.czas_min_otwarcia;
    } else {
        realPos = storage.czas_otwarcia - storage.czas_min_otwarcia;
    }
    totalWorkTime = 0;
    goto normal;

force:
    setDirection(dirRequested);
    WAIT_UNTIL(false, {
        signalPos = 256 * std::max(
            std::min(realPos, (storage.czas_otwarcia - storage.czas_min_otwarcia)),
            storage.czas_min_otwarcia);
    });
}

int Zawor::getRequestedDir()
{
    if (signalValue == 0 || !switchTimer.ready()) return 0;

    int lastSwitchTime = switchTimer.overtime();

    int limit;
    if (lastSwitchTime < 2 * storage.czas_przerwy + storage.czas_pracy_max) {
        limit = storage.czas_pracy_max;
    } else {
        limit = storage.czas_pracy_min;
    }

    int predicted = signalPos + signalValue * (storage.czas_pracy_max * limit) / (storage.czas_pracy_max + storage.czas_przerwy);
    int diff = predicted / 256 - realPos;
    if (signalValue > 0 && diff > limit) return +1;
    if (signalValue < 0 && diff < -limit) return -1;
    return 0;
}

bool Zawor::posUpdateStop(int direction)
{
    if (direction == 0) return true;
    int diff = signalPos - 256 * realPos;
    if (direction < 0 && diff >= 0) return true;
    if (direction > 0 && diff <= 0) return true;
    Time::schedule((std::abs(diff) + 767) / 256);
    return false;
}

bool Zawor::longWorkSyncReq(int direction)
{
    if (direction == 0 || totalWorkTime < ZAW_RESET_WORK_TIME_MUL * storage.czas_otwarcia) return false;
    if (direction < 0 && signalPos == 256 * storage.czas_min_otwarcia) return true;
    if (direction > 0 && signalPos == 256 * (storage.czas_otwarcia - storage.czas_min_otwarcia)) return true;
    return false;
}

bool Zawor::setDirection(int direction)
{
    if (direction == expectedDirection)
        return (direction == realDirection);
    expectedDirection = direction;
    if (!switchTimer.ready())
        return (direction == realDirection);
    if (direction != realDirection)
        setRealDirection();
    return (direction == realDirection);
}

void Zawor::setRealDirection()
{
    switchTimer.set(ZAW_PRZERWA_PRZEK);
    if (realDirection * expectedDirection < 0 || expectedDirection == 0) { // stop also if signs are different
        Relay::set(relayOn, false);
        Relay::powerOff(relayPlus);
        realDirection = 0;
    } else {
        if (expectedDirection > 0) {
            Relay::set(relayPlus, true);
            realDirection = +1;
        } else {
            Relay::set(relayPlus, false);
            realDirection = -1;
        }
        Relay::set(relayOn, true);
    }
}

void Zawor::reset(int newDirection, bool full) {
    sendEvent((full ? RESET_FULL : RESET) | (newDirection < 0 ? FLAG_MINUS : FLAG_PLUS));
}

bool Zawor::ready() {
    return (activeEvent & 0xFF) == NORMAL;
}

void Zawor::force(int newDirection) {
    if (newDirection != 0) {
        sendEvent(FORCE_START | (newDirection < 0 ? FLAG_MINUS : FLAG_PLUS));
    } else {
        sendEvent(FORCE_STOP);
    }
}

void Zawor::signal(int value) {
    signalValue = std::min(std::max(value, -512), +512);
}

bool Zawor::isFullyOpen() {
    return ready() && (signalPos == 256 * (storage.czas_otwarcia - storage.czas_min_otwarcia));
}

Zawor Zawor::powrotu(GLOBAL::storage.zaw_powrotu, Relay::ZAW_POWR, Relay::ZAW_POWR_PLUS);
Zawor Zawor::podl1(GLOBAL::storage.zaw_podl1, Relay::ZAW_PODL1, Relay::ZAW_PODL1_PLUS);
Zawor Zawor::podl2(GLOBAL::storage.zaw_podl2, Relay::ZAW_PODL2, Relay::ZAW_PODL2_PLUS);
Zawor* Zawor::podl[2] = { &podl1, &podl2 };

