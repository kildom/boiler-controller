
#include "global.hh"
#include "emergency.hh"
#include "states.hh"
#include "podl.hh"
#include "utils.hh"
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
    return (storage.pelletCwu || storage.pelletDom)
        && !emergencyDisable.pellet
        && !(emergencyDisable.cwu && emergencyDisable.podl[0] && emergencyDisable.podl[1]);
}

bool selectedElek() {
    return !selectedPellet()
        && (/*storage.elekCwu || */storage.elekDom)
        && !emergencyDisable.elek
        && !(/*emergencyDisable.cwu &&*/ emergencyDisable.podl[0] && emergencyDisable.podl[1]);
}

bool cwuHeatEnabled() {
    return /*storage.elekCwu || */storage.pelletCwu;
}

bool cwuHeat(bool forceMax) {
    auto cwuTemp = Temp::cwu();

    if (cwuTemp == Temp::INVALID) {
        emergencyDisable.cwu = true; // TODO: in function, show ERR if disabled.
    }

    int min = storage.cwuTempMin;
    int max = storage.cwuTempMax;
    if (forceMax) {
        min = storage.cwuTempMax;
        max = storage.cwuTempMax + 50;
    }

    if (emergencyDisable.cwu) {
        return false;
    } else if (cwuTemp >= storage.cwuTempMax) {
        storage.cwuHeatState = false;
    } else if (cwuTemp < storage.cwuTempMin) {
        storage.cwuHeatState = true;
    }

    return storage.cwuHeatState && cwuHeatEnabled();
}

bool roomHeatEnabled() {
    return storage.elekDom || storage.pelletDom;
}



bool roomHeat() {
    if (emergencyDisable.podl[0] && emergencyDisable.podl[1]) {
        return false;
    } else {
        return Input::heatRoom() && roomHeatEnabled();
    }
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
    Relay::pompa_cwu(false);
    Relay::buzzer(false);
    Zawor::powrotu.reset(-1, true);
    Podl::podl1.pompa(false);
    Podl::podl2.pompa(false);
    Podl::podl1.reset(-1, true);
    Podl::podl2.reset(-1, true);

update:
    if (Zawor::powrotu.ready() && Podl::podl1.ready() && Podl::podl2.ready()) {
        // TODO: sprawdzenie połączeń: zawory, pompy, czujniki (czujniki może powinny być sprawdzane na bierząco)
        setState(stateGlobalIdle);
    }
}
