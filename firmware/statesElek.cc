
#include "states.hh"
#include "statesElek.hh"

static void stateElekRoomStopping(Stage stage);
static void stateElekRoomWorking(Stage stage);
static void stateElekRoomIdle(Stage stage);

static bool stateElekParent(Stage stage)
{
    PARENT_STATE(nullptr);

enter:
update:

    if (!selectedElek()) {
        setState(stateStartup);
        return false;
    }

    return true;
}

static void stateElekRoomStopping(Stage stage)
{
    STATE(stateElekParent, "Kocioł ekek. - ogrz. domu (wyłączanie)");

enter:
    Relay::elek(false);

update:
    if (stateTime > storage->elekOffTime) {
        setState(stateElekRoomIdle);
    }
}

static void stateElekRoomWorking(Stage stage)
{
    STATE(stateElekParent, "Kocioł ekek. - ogrz. domu (działa)");

enter:
    Relay::elek(true);
    Relay::pompa_podl1(true);
    Relay::pompa_podl2(true);

update:
    if (!roomHeat()) {
        setState(stateElekRoomStopping);
    }
}

static void stateElekRoomIdle(Stage stage)
{
    STATE(stateElekParent, "Kocioł ekek. - ogrz. domu (oczekiwanie)");

enter:
    Relay::elek(false);
    Relay::pompa_podl1(false);
    Relay::pompa_podl2(false);

update:    
    if (roomHeat()) {
        setState(stateElekRoomWorking);
    }
}

void stateElekIdle(Stage stage)
{
    STATE(stateElekParent, "Kocioł ekektryczny - nieaktywny.");

enter:
    // Zatrzymaj wszystko
    Relay::elek(false);
    Relay::pompa_podl1(false);
    Relay::pompa_podl2(false);
    Relay::pompa_cwu(false);
    Zawor::podl1.reset(+1, false);
    Zawor::podl2.reset(+1, false);

update:
    if (Zawor::podl1.ready() && Zawor::podl2.ready() && stateTime > (uint64_t)storage->elekStartupTime) {
        if (roomHeat()) {
            setState(stateElekRoomIdle); // different state if C.W.U implemented
        } else if (cwuHeat()) {
            //setState(stateElekCWU);
        }
    }
}
