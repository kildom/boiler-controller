
#include "deque.hh"

struct EmergencyDisable {
    bool pellet; // kocioł na pellet nie nagrzewa się na czas, czunik pieca lub powrotu nie działa, pompa nie działa, zawór nie działa.
    bool elek; // kocioł elektryczny nie nagrzewa się na czas, czujnik nie dziła
    bool cwu; // czujnik lub pompa cwu nie działa
    bool podl1; // czujnik lub pompa podl 1 nie działa
    bool podl2; // czujnik lub pompa podl 2 nie działa
};

extern EmergencyDisable emergencyDisable;

void emergencyUpdate();
