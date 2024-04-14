#ifndef _EMERGENCY_HH_
#define _EMERGENCY_HH_

#include "global.hh"
#include "deque.hh"

struct EmergencyDisable {
    bool pellet; // kocioł na pellet nie nagrzewa się na czas, czunik pieca lub powrotu nie działa, pompa nie działa, zawór nie działa.
    bool elek; // kocioł elektryczny nie nagrzewa się na czas, czujnik nie dziła
    bool cwu; // czujnik lub pompa cwu nie działa
    bool podl[2]; // czujnik lub pompa podl nie działa
};

extern EmergencyDisable emergencyDisable;

void emergencyUpdate();

#endif
