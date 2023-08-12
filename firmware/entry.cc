
#include "lowlevel.h"
#include "log.hh"
#include "relays.hh"
#include "zawor.hh"
#include "storage.hh"
#include "diag.hh"

typedef enum Stage {
    STAGE_ENTER,
    //STAGE_LEAVE, // is it necessary?
    STAGE_UPDATE,
} Stage;


static Zawor zaw_powrotu(storage->zaw_powrotu, Relay::ZAW_POWR, Relay::ZAW_POWR_PLUS);
static Zawor zaw_podl1(storage->zaw_podl1, Relay::ZAW_PODL1, Relay::ZAW_PODL1_PLUS);
static Zawor zaw_podl2(storage->zaw_podl2, Relay::ZAW_PODL2, Relay::ZAW_PODL2_PLUS);

typedef void (*StateFunction)(Stage stage);

static StateFunction currentState = NULL;

static const char* currentStateName = "[init]";

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
    state(STAGE_ENTER);
}

void setStateMessage(const char* text)
{
    INF("STATE: %s", text);
}

void stateStartup(Stage stage)
{
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
        /*if (selectedPiec()) {
            setState(statePiecIdle);
        } else if (selectedElek()) {
            setState(stateElekIdle);
        } else*/ {
            setStateMessage("Brak wybranych zadań. CO i CWU wyłączone.");
        }
    }
}

void update_all()
{
    Time::update_start();
    Diag::update();
    zaw_powrotu.update();
    zaw_podl1.update();
    zaw_podl2.update();
}

void startup_event()
{
    INF("Controller startup");
    Storage::init();
    update_all();
    setState(stateStartup);
}

void button_event(int index, bool state)
{

}

void timeout_event()
{
    update_all();
    currentState(STAGE_UPDATE);
}

void comm_event(uint8_t* data, int size)
{
    
}
