
#include "coroutines.hh"
#include "states.hh"
#include "statesElek.hh"

// TODO: Objekt do obsługi podlogówki:
// - z obsługą emergency disable pod spodem (zawór zamknięty, pompka nie działa, zraca fałszywą temperature: wartość zadana)
// - z zapamiętywaniem stanu, żeby nie trzeba było robić bezsensownych resetów
// - z bezpiecznym przywracaniem stanu, gdy fault usunięty:
//    - jeżeli fullOpen - reset(+1, true)
//    - jeżeli fullClose - reset(-1, true)
//    - jeżeli signal - reset(-1, true)
//    - po resecie
//       - włącz pomkę, jeżeli trzeba
//       - przywóć zwracanie rzeczywistej temeratury
struct {
    void fullOpen(); // just set state if emergency disable
    void fullClose(); // just set state if emergency disable
    void signal(int value); // just set state if emergency disable
    bool ready(); // zawsze true, gdy emergency disable
    void pompa(bool); // pompa zawsze wyłączona, gdy emergency disable
    bool isFullOpen(); // zależy od stanu, gdy emergency disable
    int temp(); // zawsze równa zadanej, gdy emergency disable
    bool isDisabled();
};

void stateElek(Stage stage)
{
    static void* resumeLabel;
    static Timer timer;

    STATE(nullptr, "Kocioł ekektryczny - rozpoczynanie");

enter:
    resumeLabel = nullptr;
    Relay::elek(false);
    Relay::pompa_podl1(false); // TODO: emergency disable podl1/podl2
    Relay::pompa_podl2(false);
    Relay::pompa_cwu(false);
    Zawor::podl1.reset(+1, false);
    Zawor::podl2.reset(+1, false);

    WAIT_FOR(storage->elekStartupTime);
    WAIT_UNTIL(Zawor::podl1.ready() && Zawor::podl2.ready());

    while (true) {
        setStateMessage("Kocioł ekektryczny - oczekiwanie");
        WAIT_UNTIL(roomHeat());
        Relay::elek(true);
        Relay::pompa_podl1(true);
        Relay::pompa_podl2(true);
        setStateMessage("Kocioł ekektryczny - grzanie");
        WAIT_UNTIL(!roomHeat());
        Relay::elek(false);
        setStateMessage("Kocioł ekektryczny - wyłączanie");
        WAIT_FOR(storage->elekOffTime);
        Relay::pompa_podl1(false);
        Relay::pompa_podl2(false);
    }

update:
    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }
    goto *resumeLabel;
}
