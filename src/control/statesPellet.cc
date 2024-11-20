
#include <algorithm>

#include "global.hh"
#include "log.hh"
#include "emergency.hh"
#include "coroutines.hh"
#include "podl.hh"
#include "states.hh"
#include "statesPellet.hh"

enum HeatMode {
    NONE,
    CWU,
    ROOM,
};

static DelayOffCondition delayOff;

static bool pelletRoomHeat()
{
    return delayOff.get(roomHeat(), storage.elekMinWorkTime);
}

static HeatMode getHeatMode()
{
    if (cwuHeat() || (!pelletRoomHeat() && cwuHeat(true))) {
        return CWU;
    } else if (pelletRoomHeat()) {
        return ROOM;
    } else {
        return NONE;
    }
}

static bool coolDownWithCwu()
{
    return Temp::piecPelet() > Temp::cwu() + storage.coolDownMarginCwu
        && Temp::cwu() < storage.cwuTempMax + storage.cwuTempAddCoolDown
        && !roomHeatEnabled()
        && !emergencyDisable.cwu
        && !cwuHeat()
        && !roomHeat();
}

static bool coolDownWithPodl()
{
    return !coolDownWithCwu()
        && Temp::piecPelet() > storage.zaw_podl1.temp
        && Temp::piecPelet() > storage.zaw_podl2.temp
        && (!emergencyDisable.podl[0] || !emergencyDisable.podl[1])
        && !cwuHeat()
        && !roomHeat();
}

/**
 * @param diff              [°C/100] Różnica temperatur
 * @param hist              [°C/100] Histereza +/-
 * @param proportionalDiff  [°C/100] Różnica temperatur, poniżej której sterowanie zaczyna być proporcjonalny do różnicy
 * @returns                 [%]      Procent maksymalnego sterowania, ze znakiem, może przekraczać 100%
*/
int calcProportional(int temp, Zawor& zaw) {
    int diff = zaw.storage.temp - temp;
    int hist = zaw.storage.hist;
    int proportionalDiff = zaw.storage.proportionalDiff;
    int sign = diff < 0 ? -1 : 1;
    diff = std::max(0, diff * sign - hist) * 100 / proportionalDiff;
    return diff * sign;
}

template <typename T1, typename T2, typename T3>
auto range(T1 value, T2 min, T3 max) {
    return std::min(max, std::max(min, value));
}

static void checkFaults()
{
    static DelayOnCondition cond;
    
    // TODO: Czy tu nie trzeba też sprawdzić, czy kocioł jest rzeczywiście włączony
    if (cond.get(Temp::piecPelet() < storage.zaw_powrotu.temp, storage.pelletMaxCzasPostoju)) {
        ERR("Kocioł na pellet ostygł i nie nagrzewa się.");
        emergencyDisable.pellet = true; // TODO: set fault
    }
}

static void controlCwu()
{
    auto temp = Temp::piecPowrot();
    int prop = calcProportional(temp, Zawor::powrotu);
    prop = range(prop, -100, 100);
    Zawor::powrotu.signal(prop);
    Relay::pompa_cwu(temp >= storage.zaw_powrotu.critical);
    checkFaults();
}

static void controlPodl(bool ochronaPowrotu)
{
    int temp1 = Podl::podl1.temp();
    int temp2 = Podl::podl2.temp();
    int tempPowr = Temp::piecPowrot();

    // Sterowanie proporcjonalne dla każdego zaworu
    int propT1 = calcProportional(temp1, Zawor::podl1);
    int propT2 = calcProportional(temp2, Zawor::podl2);

    // Uwzględnij sterowanie proporcjonalne dla ochrony powrotu
    if (ochronaPowrotu) {
        int propR = -calcProportional(tempPowr, Zawor::powrotu);
        propT1 = std::min(propT1, propR);
        propT2 = std::min(propT2, propR);
    }

    // Przytnij sterowanie do maksymanych wortości
    propT1 = range(propT1, -100, 100);
    propT2 = range(propT2, -100, 100);

    // Dodaj sterowanie balansujące oba zawory
    int minBalancingTemp = storage.tempOtoczenia + storage.zaw_podl1.hist + storage.zaw_podl2.hist;
    if (storage.zaw_podl1.temp > minBalancingTemp
        && storage.zaw_podl2.temp > minBalancingTemp
        && !Podl::podl1.isDisabled()
        && !Podl::podl2.isDisabled())
    {
        // Skaluj temperatury tak, żeby były porównywalne
        int mid = (storage.zaw_podl1.temp + storage.zaw_podl2.temp) / 2 - storage.tempOtoczenia;
        int scaled1 = (temp1 - storage.tempOtoczenia) * mid / (storage.zaw_podl1.temp - storage.tempOtoczenia);
        int scaled2 = (temp2 - storage.tempOtoczenia) * mid / (storage.zaw_podl2.temp - storage.tempOtoczenia);
        // Jeżeli różnica powyżej histerezy, dodaj mały stały synał do jednego, a odejmij go od drugiego
        int diff = scaled1 - scaled2;
        int minHist = std::min(storage.zaw_podl1.hist, storage.zaw_podl2.hist);
        int balance = (diff < -minHist) ? storage.sygnalBalansujacy : (diff > minHist) ? -storage.sygnalBalansujacy : 0;
        propT1 += balance;
        propT2 -= balance;
    }

    Podl::podl1.signal(propT1);
    Podl::podl2.signal(propT2);

    static DelayOffCondition delayOffPowr;
    bool powrCritical = delayOffPowr.get(tempPowr < storage.zaw_powrotu.critical, storage.powrCriticalOffTime);

    Podl::podl1.pompa(!ochronaPowrotu || !powrCritical); // TODO: Czy zawory nadal są sterowane, jeżeli tak, to ponownym włączeniu pomp może wystąpić zbyt duża temeratura podł.
    Podl::podl2.pompa(!ochronaPowrotu || !powrCritical);

    checkFaults();
}

void statePellet(Stage stage)
{
    static void* resumeLabel;
    static Timer timer;

    STATE(nullptr, "Pellet - rozpoczynanie");

enter:
    resumeLabel = nullptr;

    while (true) {
        Relay::paliwo(false);
        Relay::piec(false);
        Relay::pompa_powr(false);
        Relay::pompa_cwu(false);
        Zawor::powrotu.reset(+1, false);
        Podl::podl1.pompa(false);
        Podl::podl2.pompa(false);
        Podl::podl1.reset(-1, false);
        Podl::podl2.reset(-1, false);

        setStateMessage("Pellet - rozpoczynanie");
        WAIT_UNTIL(Zawor::powrotu.ready() && Podl::podl1.ready() && Podl::podl2.ready());

        setStateMessage("Pellet - oczekiwanie");
        WAIT_UNTIL(roomHeat() || cwuHeat());

        setStateMessage("Pellet - rozpalanie");
        Relay::piec(true);
        Relay::paliwo(true);
        Relay::pompa_powr(true);
        timer.set(storage.pelletHeatUpMaxTime);
        WAIT_UNTIL((!roomHeat() && !cwuHeat()) || timer.ready() || Temp::piecPowrot() > storage.zaw_powrotu.temp);

        if (timer.ready()) {
            ERR("Piec na pellet nie rozgrzał się na czas!");// TODO: set fault
            emergencyDisable.pellet = true;
            goto exit_state;
        }

        delayOff.triggerNow(storage.pelletMinWorkTime);

        while (Relay::piec()) {

            static HeatMode mode;

            mode = getHeatMode();

            if (mode == CWU) {
    
                setStateMessage("Pellet - grzanie C.W.U. - zawory");
                if (Temp::piecPelet() > storage.pelletRozgrzanyTemp) {
                    Zawor::powrotu.reset(-1, false); // przy rozgrzanym piecu lepiej zaczynać od zimniejszego powrotu, żeby nie przegrzać pieca
                } else {
                    Zawor::powrotu.reset(+1, false); // przy nie całkowicie rozgrzanym piecu lepiej zaczynać od ciepłego powrotu w celu ochrony powrotu
                }
                WAIT_UNTIL(Zawor::powrotu.ready());

                setStateMessage("Pellet - grzanie C.W.U.");
                Relay::pompa_cwu(true);
                WAIT_UNTIL(getHeatMode() != CWU, {
                    controlCwu();
                });

                if (getHeatMode() == NONE) {
                    Relay::piec(false);
                    Relay::paliwo(false);
                    setStateMessage("Pellet - wygaszanie ognia");
                    WAIT_FOR(storage.pelletCzasWygasania, {
                        controlCwu();
                    });

                    if (coolDownWithCwu()) {
                        setStateMessage("Pellet - pozostałości ciepła do C.W.U.");
                        Zawor::powrotu.reset(-1, false);
                        Relay::pompa_cwu(true);
                        WAIT_UNTIL(!coolDownWithCwu());
                    }
                }
            
                Relay::pompa_cwu(false);

            } else if (mode == ROOM) {

                setStateMessage("Pellet - grzanie pomieszczeń");
                Zawor::powrotu.reset(-1, false);
                WAIT_UNTIL(getHeatMode() != ROOM, {
                    controlPodl(true);
                });

                if (getHeatMode() == NONE) {
                    Relay::piec(false);
                    Relay::paliwo(false);
                    setStateMessage("Pellet - wygaszanie ognia");
                    WAIT_FOR(storage.pelletCzasWygasania, {
                        controlPodl(true);
                    });
                    setStateMessage("Pellet - pozostałości ciepła do C.O.");
                    WAIT_UNTIL(!coolDownWithPodl(), {
                        controlPodl(false);
                    });
                }

                Podl::podl1.reset(-1, false);
                Podl::podl2.reset(-1, false);
                Podl::podl1.pompa(false);
                Podl::podl2.pompa(false);

            } else {

                Relay::piec(false);
                Relay::paliwo(false);

            }
        }

        if (coolDownWithPodl()) {
            setStateMessage("Pellet - pozostałości ciepła do C.O.");
            Zawor::powrotu.reset(-1, false);
            WAIT_UNTIL(!coolDownWithPodl(), {
                controlPodl(false);
            });
            // TODO: albo inny warunek stopu: zawór otwarty na 100%, a temp < zadana przez więcej niż 5 min
            // można wyłączyć tylko jedną pompę, jeżeli tylko jeden zawór spełnia warunek stopu.
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

/*
 * TODO: Może kiedyś:
 * - otwarcie zaworów podłogówki na 100% (przynajmniej jeden, gdy istnieje asymetria) 
 * - sterowanie zaworem powrotu
 * - zalety: mniej rurek ma wysoką temperaturę, więc mniej strat jest w kotłowni
 */
