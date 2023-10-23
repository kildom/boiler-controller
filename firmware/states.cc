
#include "emergency.hh"
#include "states.hh"
#include "statesElek.hh"


static StateFunction currentState = NULL;

const char* currentStateName = "[init]";


static uint64_t stateStartTime = 0;
uint64_t stateTime = 0;


uint32_t StateFlag::stateFlags = 0;



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

bool selectedPellet() {
    return (storage->pelletCwu || storage->pelletDom)
        && !emergencyDisable.pellet
        && !(emergencyDisable.cwu && emergencyDisable.podl1 && emergencyDisable.podl2);
}

bool selectedElek() {
    return !selectedPellet()
        && (/*storage->elekCwu || */storage->elekDom)
        && !emergencyDisable.elek
        && !(emergencyDisable.cwu && emergencyDisable.podl1 && emergencyDisable.podl2);
}

bool cwuHeat(bool forceMax) {
    auto cwuTemp = Temp::cwu();

    if (cwuTemp == Temp::INVALID) {
        emergencyDisable.cwu = true; // TODO: in function, show ERR if disabled.
    }

    if (emergencyDisable.cwu) {
        return false;
    } else if (cwuTemp >= storage->cwuTempMax) {
        storage->cwuHeatState = false;
    } else if (forceMax || cwuTemp < storage->cwuTempMin) {
        storage->cwuHeatState = true;
    }

    return storage->cwuHeatState && (/*storage->elekCwu || */storage->pelletCwu);
}


bool roomHeat() {
    if (emergencyDisable.podl1 && emergencyDisable.podl2) {
        return false;
    }

    if (Input::heatRoom()) { // TODO: always false if all room heating disabled
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

void updateState()
{
    stateTime = Time::time - stateStartTime;
    currentState(STAGE_UPDATE);
}

static void stateGlobalIdle(Stage stage)
{
    STATE(nullptr, "Brak wybranych zadań. CO i CWU wyłączone.");

enter:
update:
    if (selectedPellet()) {
        //setState(statePiecIdle);
    } else if (selectedElek()) {
        setState(stateElek);
    }
}

void stateStartup(Stage stage)
{
    static const StateFlag messageShown(0);

    STATE(nullptr, "Rozpoczynanie pracy. Reset zaworów.");

enter:
    Relay::paliwo(false);
    Relay::piec(false);
    Relay::elek(false);
    Relay::pompa_powr(false);
    Relay::pompa_podl1(false);
    Relay::pompa_podl2(false);
    Relay::pompa_cwu(false);
    Relay::buzzer(false);
    Zawor::powrotu.reset(-1, true);
    Zawor::podl1.reset(-1, true);
    Zawor::podl2.reset(-1, true);

update:
    if (Zawor::powrotu.ready() && Zawor::podl1.ready() && Zawor::podl2.ready()) {
        setState(stateGlobalIdle);
    }
}
