#ifndef _INPUTS_H_
#define _INPUTS_H_

#include "global.hh"
#include "controlInterface.hh"

class Input {
public:

    enum Index {
        HEAT_ROOM = 0,
        PELLET_PODAWANY = 1,
        INPUTS_COUNT = 2,
    };

    static inline int heatRoom() { return get(HEAT_ROOM); }
    static inline int pelletPodawany() { return get(PELLET_PODAWANY); }

    static int get(Index index) { return input(index); }

};

#endif
