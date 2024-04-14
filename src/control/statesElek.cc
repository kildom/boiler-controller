
#include "global.hh"
#include "coroutines.hh"
#include "states.hh"
#include "podl.hh"
#include "utils.hh"
#include "statesElek.hh"

static bool elekRoomHeat() {
    static DelayOffCondition delayOff;
    return delayOff.get(roomHeat(), storage.elekMinWorkTime);
}

void stateElek(Stage stage)
{
    static void* resumeLabel;
    static Timer timer;

    STATE(nullptr, "Kocioł ekektryczny - rozpoczynanie");

enter:
    resumeLabel = nullptr;
    Relay::elek(false);
    Relay::pompa_cwu(false);
    Podl::podl1.pompa(false);
    Podl::podl2.pompa(false);
    Podl::podl1.reset(+1, false);
    Podl::podl2.reset(+1, false);

    WAIT_FOR(storage.elekStartupTime);
    WAIT_UNTIL(Podl::podl1.ready() && Podl::podl2.ready());

    while (true) {
        setStateMessage("Kocioł ekektryczny - oczekiwanie");
        WAIT_UNTIL(elekRoomHeat());
        Relay::elek(true);
        Podl::podl1.pompa(true);
        Podl::podl2.pompa(true);
        setStateMessage("Kocioł ekektryczny - grzanie");
        WAIT_UNTIL(!elekRoomHeat());
        Relay::elek(false);
        setStateMessage("Kocioł ekektryczny - wyłączanie");
        WAIT_FOR(storage.elekOffTime);
        Podl::podl1.pompa(false);
        Podl::podl2.pompa(false);
    }

update:
    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }
    goto *resumeLabel;
}
