
#include <algorithm>

#include "log.hh"
#include "emergency.hh"
#include "coroutines.hh"
#include "states.hh"
#include "statesPellet.hh"


void statePellet(Stage stage)
{
    static void* resumeLabel;
    static Timer timer;
    static uint64_t t;

    STATE(nullptr, "Pellet - rozpoczynanie");

enter:
    resumeLabel = nullptr;

    while (true) {
        Relay::paliwo(false);
        Relay::piec(false);
        Relay::pompa_powr(false);
        Relay::pompa_podl1(false);
        Relay::pompa_podl2(false);
        Relay::pompa_cwu(false);
        Zawor::powrotu.reset(+1, false);
        Zawor::podl1.reset(-1, false);
        Zawor::podl2.reset(-1, false);

        setStateMessage("Pellet - rozpoczynanie");
        WAIT_UNTIL(Zawor::powrotu.ready() && Zawor::podl1.ready() && Zawor::podl2.ready());

        setStateMessage("Pellet - oczekiwanie");
        WAIT_UNTIL(roomHeat() || cwuHeat());

        setStateMessage("Pellet - rozpalanie");
        Relay::piec(true);
        Relay::paliwo(true);
        Relay::pompa_powr(true);
        storage->roomHeatEnd = Time::time + storage->pelletMinWorkTime;
        timer.set(storage->pelletHeatUpMaxTime);
        WAIT_UNTIL((!roomHeat() && !cwuHeat()) || timer.ready() || Temp::piecPowrot() > storage->zaw_powrotu.temp);

        if (timer.ready()) {
            ERR("Piec na pellet nie rozgrzał się na czas!");// TODO: set fault
            emergencyDisable.pellet = true;
            goto exit_state;
        }

        while (true) {
            if (cwuHeat() || (!roomHeat() && cwuHeat(true))) {
    
                setStateMessage("Pellet - grzanie C.W.U. - zawory");
                Relay::pompa_podl1(false);
                Relay::pompa_podl2(false);  
                Zawor::podl1.reset(-1, false);
                Zawor::podl2.reset(-1, false);
                Zawor::powrotu.reset(-1, false);
                WAIT_UNTIL(Zawor::powrotu.ready() && Zawor::podl1.ready() && Zawor::podl2.ready());

                setStateMessage("Pellet - grzanie C.W.U.");
                Relay::pompa_cwu(true);
                WAIT_UNTIL(!cwuHeat(true)); // TODO: control zaw. powrotu i pompą C.W.U.
    
                Relay::pompa_cwu(false);

            } else if (roomHeat()) {

                setStateMessage("Pellet - grzanie pomieszczeń");
                Relay::pompa_podl1(false);
                Relay::pompa_podl2(false);  
                Zawor::powrotu.reset(-1, false);
                Zawor::podl1.reset(-1, false);
                Zawor::podl2.reset(-1, false);
                // TODO: reset control of zaw. podl
                t = Time::time;
                WAIT_UNTIL(!roomHeat() || cwuHeat()); // TODO: control zaw. podl
                // ...
            } else {
                break;
            }
        }

        Relay::piec(false);
        Relay::paliwo(false);

        // TODO: Control last mode (cwu/room) for next 5 minutes.

        if (Temp::piecPelet() > Temp::cwu() + storage->coolDownMarginCwu && Temp::cwu() < storage->cwuTempMax + storage->cwuTempAddCoolDown) {
            // TODO: pozostałości ciepła do C.W.U. powinny być tylko w lato
            setStateMessage("Pellet - pozostałości ciepła do C.W.U.");
            Zawor::powrotu.reset(-1, false);
            Relay::pompa_cwu(true);
            WAIT_UNTIL(!(Temp::piecPelet() > Temp::cwu() + storage->coolDownMarginCwu && Temp::cwu() < storage->cwuTempMax + storage->cwuTempAddCoolDown));
            // TODO: stop cooling down if roomHeat() || cwuHeat()
        }

        if (Temp::piecPelet() > storage->zaw_podl1.temp && Temp::piecPelet() > storage->zaw_podl2.temp) {
            setStateMessage("Pellet - pozostałości ciepła do C.O.");
            // TODO: control zaw. podl bez ochrony powrotu
            WAIT_UNTIL(!(Temp::piecPelet() > storage->zaw_podl1.temp && Temp::piecPelet() > storage->zaw_podl2.temp));
            // TODO: albo inny warunek stopu: zawór otwarty na 100%, a temp < zadana przez więcej niż 5 min
            // można wyłączyć tylko jedną pompę, jeżeli tylko jeden zawór spełnia warunek stopu.
            // TODO: stop cooling down if roomHeat() || cwuHeat()
        }
    }

exit_state:
    setState(stateStartup);
    return;

update:
    if (!selectedPellet()) {
        setState(stateStartup);
        return;
    }
    goto *resumeLabel;
}
