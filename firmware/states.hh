#ifndef _STATES_HH_
#define _STATES_HH_

#include "lowlevel.hh"
#include "log.hh"
#include "relays.hh"
#include "zawor.hh"
#include "storage.hh"
#include "diag.hh"
#include "temp.hh"
#include "utils.hh"
#include "inputs.hh"

typedef enum Stage {
    STAGE_ENTER,
    //STAGE_LEAVE, // is it necessary?
    STAGE_UPDATE,
} Stage;

typedef void (*StateFunction)(Stage stage);

class StateFlag {
private:
    static uint32_t stateFlags;
public:
    uint32_t mask;
    StateFlag(int index) : mask(1 << index) {}
    uint32_t get() const { return stateFlags & mask; }
    bool once() const { auto ret = get(); set(); return !ret; }
    void set() const { stateFlags |= mask; }
    void reset() const { stateFlags &= ~mask; }
    operator bool() const { return stateFlags & mask ? true : false; }
    static void clearAll() { stateFlags = 0; }
};

extern uint64_t stateTime;
extern const char* currentStateName;


#define setState(stateName) do { \
    DBG("STATE: %s => %s  (in %s)", currentStateName, &(#stateName)[5], __func__); \
    currentStateName = &(#stateName)[5]; \
    setStateImpl(stateName); \
} while (0);


template<typename T>
struct _CallParentHelper;

template<>
struct _CallParentHelper<nullptr_t>
{
    static bool call(nullptr_t _, Stage stage) {
        return true;
    }
};

template<>
struct _CallParentHelper<void(Stage stage)>
{
    static bool call(void parent(Stage stage), Stage stage) {
        parent(stage);
        return true;
    }
};

template<>
struct _CallParentHelper<bool(Stage stage)>
{
    static bool call(bool parent(Stage stage), Stage stage) {
        return parent(stage);
    }
};


#define STATE(parent, message) do { \
    if (!_CallParentHelper<decltype(parent)>::call(parent, stage)) return; \
    if (stage == STAGE_ENTER) {\
        setStateMessage(message); \
        goto enter; \
    } else { \
        goto update; \
    } \
} while (0)

#define PARENT_STATE(parent) do { \
    if (!_CallParentHelper<decltype(parent)>::call(parent, stage)) return false; \
    if (stage == STAGE_ENTER) {\
        goto enter; \
    } else { \
        goto update; \
    } \
} while (0)

//#define STATE_WITH_LEAVE(parent) - if leave implemented

void setStateImpl(StateFunction state);
void setStateMessage(const char* text);

bool selectedPellet();
bool selectedElek();
bool cwuHeat(bool forceMax = false);
bool roomHeat();

void updateState();

void stateStartup(Stage stage);

#endif
