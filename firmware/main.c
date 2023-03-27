
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define LOG_ENABLE 1
#define ERR(text, ...) printf("ERR  " text "\n", ##__VA_ARGS__)
#define LOG(text, ...) printf("LOG  " text "\n", ##__VA_ARGS__)
#define DBG(text, ...) printf("DBG  " text "\n", ##__VA_ARGS__)

typedef int64_t timestamp;

typedef enum Stage {
    STAGE_ENTER,
    //STAGE_LEAVE, // TODO: is it necessary?
    STAGE_UPDATE,
} Stage;

typedef enum RelayName {
    RELAY_PALIWO,
    RELAY_PIEC,
    RELAY_ELEK,
    RELAY_POMPA_POWR,
    RELAY_ZAW_POWR_ON,
    RELAY_ZAW_POWR_PLUS,
    RELAY_ZAW_PODL1_ON,
    RELAY_ZAW_PODL1_PLUS,
    RELAY_POMPA_PODL1,
    RELAY_ZAW_PODL2_ON,
    RELAY_ZAW_PODL2_PLUS,
    RELAY_POMPA_PODL2,
    RELAY_POMPA_CWU,
} RelayName;

typedef enum ValveName {
    VALVE_POWR,
    VALVE_PODL1,
    VALVE_PODL2,
} ValveName;

typedef void (*StateFunction)(Stage stage);

static StateFunction currentState = NULL;

#if LOG_ENABLE

static const char* currentStateName = "[undefined]";

#define setState(stateName) do { \
    LOG("STATE: %s => %s  (in %s)", currentStateName, #stateName, __func__); \
    setStateImpl(stateName); \
    currentState = stateName; \
} while (0);

#else

#define setState setStateImpl

#endif

timestamp time;

void setStateImpl(StateFunction state)
{
    /*if (currentState != NULL) {
        currentState(STAGE_LEAVE);
    }*/
    currentState = state;
    state(STAGE_ENTER);
}

void globalLogic()
{

}

void update()
{
    globalLogic();
    currentState(STAGE_UPDATE);
}

void relay(RelayName name, bool value)
{
    printf("Relay %d: %s\n", name, value ? "ON" : "OFF");
}

void valve_reset(ValveName name, int direction)
{
    printf("Valve %d: reset\n", name);
}

typedef enum InputName {
    INPUT_SELECT_PIEC,
    INPUT_SELECT_ROOM,
    INPUT_SELECT_CWU,
    INPUT_COLD_ROOM,
} InputName;

bool emergencyElek = false;

bool getInput(InputName name)
{
    return false;
}

bool selectedRoom() {
    return getInput(INPUT_SELECT_ROOM);
}

bool selectedCWU() {
    return getInput(INPUT_SELECT_CWU);
}

bool selectedPiec() {
    return getInput(INPUT_SELECT_PIEC) && !emergencyElek && (selectedRoom() || selectedCWU());
}

bool selectedElek() {
    return !selectedPiec() && (selectedRoom() || selectedCWU());
}

int cwuTemp = 1000;
int cwuTempCritical = 6500;
int cwuTempMax = 5000;
int cwuTempMin = 3500;
bool cwuColdState = false;


bool cwuCold() {
    if (cwuTemp >= cwuTempMax) {
        cwuColdState = false;
    } else if (cwuTemp < cwuTempMin) {
        cwuColdState = true;
    }
    return cwuColdState && selectedCWU();
}

static const int ROOM_COLD_START_DISABLED = -24 * 60 * 60 * 1000;
static const int PARAM_ROOM_HEAT_MIN_TIME_ELEK = 15 * 60 * 1000;
static const int PARAM_ROOM_HEAT_MIN_TIME_PIEC = 2 * 60 * 60 * 1000;

timestamp roomColdStart = ROOM_COLD_START_DISABLED;

bool roomCold() {
    if (getInput(INPUT_COLD_ROOM)) {
        if (roomColdStart < 0) {
            roomColdStart = time;
        }
        return true;
    } else {
        timestamp minTime = selectedPiec() ? PARAM_ROOM_HEAT_MIN_TIME_PIEC : PARAM_ROOM_HEAT_MIN_TIME_ELEK;
        if (time > roomColdStart + minTime) {
            roomColdStart = ROOM_COLD_START_DISABLED;
            return false;
        } else {
            return true;
        }
    }
}

bool cwuCritical() {
    return cwuTemp >= cwuTempCritical;
}

timestamp stateTime;

void podlValveUpdate(bool useReturnProtect);

void stateStartup(Stage stage);
void statePiecIdle(Stage stage);
void stateElekRoom(Stage stage);
void stateElekRoomStartup(Stage stage);


static const PARAM_ELEK_SHUTDOWN_TIME = 30 * 1000;

void stateElekCWUShutDown(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - zatrzymywanie CWU.");
        relay(RELAY_ELEK, false);
    }

    if (stateTime > PARAM_ELEK_SHUTDOWN_TIME) {
        setState(stateElekIdle);
    }
}

void stateElekCWU(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - CWU.");
        relay(RELAY_ELEK, true);
        relay(RELAY_POMPA_PODL1, false);
        relay(RELAY_POMPA_PODL2, false);
        relay(RELAY_POMPA_CWU, true);
        valve_reset(VALVE_PODL1, -1);
        valve_reset(VALVE_PODL2, -1);
    }

    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }

    if (!cwuCold()) {
        if (roomCold()) {
            setState(stateElekRoomStartup);
        } else {
            setState(stateElekCWUShutDown);
        }
    }
}

void stateElekRoomStartup(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - rozpoczynanie CO.");
        valve_reset(VALVE_PODL1, -1);
        valve_reset(VALVE_PODL2, -1);
    }
    
    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }

    if (valve_ready(VALVE_PODL1) && valve_ready(VALVE_PODL2)) {
        setState(stateElekRoom);
    } else {
        relay(RELAY_ELEK, false);
        relay(RELAY_POMPA_PODL1, false);
        relay(RELAY_POMPA_PODL2, false);
        relay(RELAY_POMPA_CWU, false);
    }
}

void stateElekRoom(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - CO.");
        relay(RELAY_ELEK, true);
        relay(RELAY_POMPA_PODL1, true);
        relay(RELAY_POMPA_PODL2, true);
        relay(RELAY_POMPA_CWU, false);
    }
    
    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }

    podlValveUpdate(false);

    if (cwuCold()) {
        setState(stateElekCWU);
    } else if (!roomCold()) {
        setState(stateElekRoomShutDown);
    }
}

void stateElekRoomShutDown(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - zatrzymywanie CO.");
        relay(RELAY_ELEK, false);
    }

    podlValveUpdate(false);

    if (stateTime > PARAM_ELEK_SHUTDOWN_TIME) {
        setState(stateElekIdle);
    }
}

static const int PARAM_ELEK_STARTUP_TIME = 4 * 60 * 1000;

void stateElekIdle(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł ekektryczny - nieaktywny.");
        // Zatrzymaj wszystko
        relay(RELAY_ELEK, false);
        relay(RELAY_POMPA_PODL1, false);
        relay(RELAY_POMPA_PODL2, false);
        relay(RELAY_POMPA_CWU, false);
        valve_reset(VALVE_PODL1, -1);
        valve_reset(VALVE_PODL2, -1);
    }

    if (!selectedElek()) {
        setState(stateStartup);
        return;
    }

    if (valve_ready(VALVE_PODL1) && valve_ready(VALVE_PODL2) && time > PARAM_ELEK_STARTUP_TIME) {
        if (cwuCold()) {
            setState(stateElekCWU);
        } else if (roomCold()) {
            setState(stateElekRoom);
        }
    }
}

void statePiecIdle(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Kocioł na pellet - nieaktywny.");
        relay(RELAY_PALIWO, false);
        relay(RELAY_PIEC, false);
        relay(RELAY_POMPA_PODL1, false);
        relay(RELAY_POMPA_PODL2, false);
        relay(RELAY_POMPA_CWU, false);
        valve_reset(VALVE_PODL1, -1);
        valve_reset(VALVE_PODL2, -1);
    }

    if (!selectedPiec()) {
        setState(stateStartup);
        return;
    }

}

void stateStartup(Stage stage)
{
    if (stage == STAGE_ENTER)
    {
        setStateMessage("Rozpoczynanie pracy. Reset zaworów.");
        relay(RELAY_PALIWO, false);
        relay(RELAY_PIEC, false);
        relay(RELAY_ELEK, false);
        relay(RELAY_POMPA_POWR, true);
        relay(RELAY_POMPA_PODL1, false);
        relay(RELAY_POMPA_PODL2, false);
        relay(RELAY_POMPA_CWU, false);
        valve_reset(VALVE_POWR, -1);
        valve_reset(VALVE_PODL1, -1);
        valve_reset(VALVE_PODL2, -1);
    }

    if (valve_ready(VALVE_POWR) && valve_ready(VALVE_PODL1) && valve_ready(VALVE_PODL2)) {
        if (selectedPiec()) {
            setState(statePiecIdle);
        } else if (selectedElek()) {
            setState(stateElekIdle);
        } else {
            setStateMessage("Brak wybranych zadań. CO i CWU wyłączone.");
        }
    }
}

int main()
{
    setState(stateStartup);
    for (time = 0; time < 10000; time++) {
        update();
    }
    return 0;
}
