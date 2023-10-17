
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
    Zawor::podl1.full(+1);
    Zawor::podl2.full(+1);

update:    
    if (Zawor::podl1.full_done() && Zawor::podl2.full_done() && roomHeat()) {
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
    Zawor::podl1.reset();
    Zawor::podl2.reset();

update:
    if (Zawor::podl1.full_done() && Zawor::podl2.full_done() && stateTime > (uint64_t)storage->elekStartupTime) {
        if (roomHeat()) {
            setState(stateElekRoomIdle); // different state if C.W.U implemented
        } else if (cwuHeat()) {
            //setState(stateElekCWU);
        }
    }
}
