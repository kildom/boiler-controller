
#include "lowlevel.hh"
#include "log.hh"
#include "relays.hh"
#include "zawor.hh"
#include "storage.hh"
#include "diag.hh"
#include "temp.hh"
#include "utils.hh"
#include "inputs.hh"


typedef enum Stage {
    STAGE_ENTER,
    //STAGE_LEAVE, // is it necessary?
    STAGE_UPDATE,
} Stage;

void stateStartup(Stage stage);


static Zawor zaw_powrotu(storage->zaw_powrotu, Relay::ZAW_POWR, Relay::ZAW_POWR_PLUS);
static Zawor zaw_podl1(storage->zaw_podl1, Relay::ZAW_PODL1, Relay::ZAW_PODL1_PLUS);
static Zawor zaw_podl2(storage->zaw_podl2, Relay::ZAW_PODL2, Relay::ZAW_PODL2_PLUS);

typedef void (*StateFunction)(Stage stage);

static StateFunction currentState = NULL;

static const char* currentStateName = "[init]";


class StateFlag {
private:
    static uint32_t stateFlags;
public:
    uint32_t mask;
    StateFlag(int index) : mask(1 << index) {}
    uint32_t get() const { return stateFlags & mask; }
    void set() const { stateFlags |= mask; }
    void reset() const { stateFlags &= ~mask; }
    operator bool() const { return stateFlags & mask ? true : false; }
    static void clearAll() { stateFlags = 0; }
};

uint32_t StateFlag::stateFlags = 0;

static uint64_t stateStartTime = 0;
static uint64_t stateTime = 0;

#define setState(stateName) do { \
    DBG("STATE: %s => %s  (in %s)", currentStateName, &(#stateName)[5], __func__); \
    setStateImpl(stateName); \
    currentStateName = &(#stateName)[5]; \
} while (0);

void setStateImpl(StateFunction state)
{
    /*if (currentState != NULL) {
        currentState(STAGE_LEAVE);
    }*/
    currentState = state;
    StateFlag::clearAll();
    stateStartTime = Time::time;
    stateTime = 0;
    state(STAGE_ENTER);
}

void setStateMessage(const char* text)
{
    INF("STATE: %s", text);
}

struct EmergencyDisable {
    bool pellet; // kocioł na pellet nie nagrzewa się na czas, czunik pieca lub powrotu nie działa, pompa nie działa, zawór nie działa.
    bool elek; // kocioł elektryczny nie nagrzewa się na czas, czujnik nie dziła
    bool cwu; // czujnik lub pompa cwu nie działa
    bool podl1; // czujnik lub pompa podl 1 nie działa
    bool podl2; // czujnik lub pompa podl 2 nie działa
};

static EmergencyDisable emergencyDisable;

static bool selectedPellet() {
    return (storage->pelletCwu || storage->pelletDom)
        && !emergencyDisable.pellet
        && !(emergencyDisable.cwu && emergencyDisable.podl1 && emergencyDisable.podl2);
}

static bool selectedElek() {
    return !selectedPellet()
        && (storage->elekCwu || storage->elekDom)
        && !emergencyDisable.elek
        && !(emergencyDisable.cwu && emergencyDisable.podl1 && emergencyDisable.podl2);
}

static bool cwuHeat() {
    auto cwuTemp = Temp::cwu();

    if (cwuTemp == Temp::INVALID) {
        emergencyDisable.cwu = true; // TODO: in function, show ERR if disabled.
    }

    if (emergencyDisable.cwu) {
        return false;
    } else if (cwuTemp >= storage->cwuTempMax) {
        storage->cwuHeatState = false;
    } else if (cwuTemp < storage->cwuTempMin) {
        storage->cwuHeatState = true;
    }

    return storage->cwuHeatState && (storage->elekCwu || storage->pelletCwu);
}


static bool roomHeat() {
    if (emergencyDisable.podl1 && emergencyDisable.podl2) {
        return false;
    }

    if (Input::heatRoom()) {
        if (storage->roomHeatEnd == 0) {
            uint64_t minTime = selectedPellet() ? storage->roomMinHeatTimePellet : storage->roomMinHeatTimeElek;
            storage->roomHeatEnd = Time::time + minTime;
            DBG("Room heat requested for %d min.", (int)minTime / 60000);
        }
        return true;
    } else if (Time::time < storage->roomHeatEnd) {
        return true;
    } else if (storage->roomHeatEnd != 0) {
        DBG("Room heat request stopped.");
        storage->roomHeatEnd = 0;
    }

    return false;
}

void stateElekIdle(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - nieaktywny.");
        // Zatrzymaj wszystko
        Relay::elek(false);
        Relay::pompa_podl1(false);
        Relay::pompa_podl2(false);
        Relay::pompa_cwu(false);
        zaw_podl1.reset();
        zaw_podl2.reset();
    }

    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }

    if (zaw_podl1.full_done() && zaw_podl2.full_done() && stateTime > (uint64_t)storage->elekStartupTime) {
        if (cwuHeat()) {
            //setState(stateElekCWU);
        } else if (roomHeat()) {
            //setState(stateElekRoom);
        }
    }
}

void stateStartup(Stage stage)
{
    static const StateFlag messageShown(0);

    if (stage == STAGE_ENTER)
    {
        setStateMessage("Rozpoczynanie pracy. Reset zaworów.");

        Relay::paliwo(false);
        Relay::piec(false);
        Relay::elek(false);
        Relay::pompa_powr(false);
        Relay::pompa_podl1(false);
        Relay::pompa_podl2(false);
        Relay::pompa_cwu(false);
        Relay::buzzer(false);
        zaw_powrotu.reset();
        zaw_podl1.reset();
        zaw_podl2.reset();
    }

    if (zaw_powrotu.full_done() && zaw_podl1.full_done() && zaw_podl2.full_done()) {
        if (selectedPellet()) {
            //setState(statePiecIdle);
        } else if (selectedElek()) {
            setState(stateElekIdle);
        } else if (!messageShown) {
            messageShown.set();
            setStateMessage("Brak wybranych zadań. CO i CWU wyłączone.");
        }
    }
}

void pre_update()
{
    Time::update_start();
    Diag::update();
    stateTime = Time::time - stateStartTime;
    zaw_powrotu.update();
    zaw_podl1.update();
    zaw_podl2.update();
}

void post_update()
{
    Storage::update();
}

void startup_event()
{
    INF("Controller startup");
    Storage::init();
    pre_update();
    setState(stateStartup);
    post_update();
}

void button_event(int index, bool state)
{

}

void timeout_event()
{
    pre_update();
    currentState(STAGE_UPDATE);
    post_update();
}

void comm_event(uint8_t* data, int size)
{
    
}
